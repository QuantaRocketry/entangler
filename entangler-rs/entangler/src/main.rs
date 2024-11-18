#![no_std]
#![no_main]

use defmt as _;
use embassy_executor::Spawner;
use embassy_rp::bind_interrupts;
use embassy_rp::gpio;
use embassy_rp::i2c;
use embassy_rp::peripherals::SPI1;
use embassy_rp::peripherals::{I2C1, PIO0, USB};
use embassy_rp::pio;
use embassy_rp::spi;
use embassy_rp::usb::{self, Driver};
use embassy_rp::Peripherals;
use embassy_sync::blocking_mutex::raw::ThreadModeRawMutex;
use embassy_sync::mutex::Mutex;
use embassy_time::{Delay, Duration, Ticker, Timer};
use embedded_hal_bus::spi::ExclusiveDevice;
use gpio::{AnyPin, Input, Level, Output};
use lora_phy::sx127x;
use lora_phy::sx127x::{Sx1276, Sx127x};
use lora_phy::LoRa;
use {defmt_rtt as _, panic_probe as _};

#[embassy_executor::task]
async fn logger_task(driver: Driver<'static, USB>) {
    embassy_usb_logger::run!(1024, log::LevelFilter::Debug, driver);
}

type LedType = Mutex<ThreadModeRawMutex, Option<Output<'static>>>;
static LED: LedType = Mutex::new(None);

bind_interrupts!(struct Irqs {
    USBCTRL_IRQ => usb::InterruptHandler<USB>;
    I2C1_IRQ => i2c::InterruptHandler<I2C1>;
    PIO0_IRQ_0 => pio::InterruptHandler<PIO0>;
});

#[embassy_executor::task(pool_size = 1)]
async fn heartbeat(name: &'static str, delay: Duration) {
    let mut ticker = Ticker::every(delay);
    loop {
        log::info!("{name} heartbeat");
        ticker.next().await;
    }
}

#[embassy_executor::task]
async fn lora_rcvr(
    mut lora_device: LoRa<
        Sx127x<
            ExclusiveDevice<spi::Spi<'static, SPI1, spi::Async>, Output<'static>, Delay>,
            lora_phy::iv::GenericSx127xInterfaceVariant<Output<'static>, Input<'static>>,
            Sx1276,
        >,
        Delay,
    >,
) {
    let sf = lora_phy::mod_params::SpreadingFactor::_10;
    let bw = lora_phy::mod_params::Bandwidth::_250KHz;
    let cr = lora_phy::mod_params::CodingRate::_4_8;
    let preamble = 8;

    let mdltn_params = {
        match lora_device.create_modulation_params(sf, bw, cr, 915_000_000) {
            Ok(mp) => mp,
            Err(err) => {
                log::info!("Radio error = {:?}", err);
                return;
            }
        }
    };

    let mut tx_pkt_params = {
        match lora_device.create_tx_packet_params(preamble, false, true, false, &mdltn_params) {
            Ok(pp) => pp,
            Err(err) => {
                log::info!("Radio error = {:?}", err);
                return;
            }
        }
    };

    let buffer = [0x01u8, 0x02u8, 0x03u8];
    loop {
        Timer::after_secs(1).await;
        log::info!("wtf");

        match lora_device
            .prepare_for_tx(&mdltn_params, &mut tx_pkt_params, 20, &buffer)
            .await
        {
            Ok(()) => {}
            Err(err) => {
                log::info!("Radio error = {:?}", err);
                continue;
            }
        };

        match lora_device.tx().await {
            Ok(()) => {
                log::info!("TX DONE");
            }
            Err(err) => {
                log::info!("Radio error = {:?}", err);
                continue;
            }
        };
    }
}

#[embassy_executor::main]
async fn main(spawner: Spawner) -> ! {
    let p = embassy_rp::init(Default::default());
    let driver = Driver::new(p.USB, Irqs);
    spawner.spawn(logger_task(driver)).unwrap();
    Timer::after_secs(1).await;
    log::info!("program started");
    let led = Output::new(AnyPin::from(p.PIN_25), Level::High);

    // inner scope is so that once the mutex is written to, the MutexGuard is dropped, thus the
    // Mutex is released
    {
        *(LED.lock().await) = Some(led);
    }

    spawner
        .spawn(heartbeat("task1", Duration::from_secs(3)))
        .unwrap();
    if let Err(_) = spawner.spawn(heartbeat("task3", Duration::from_secs(5))) {
        // SpawnError
        log::info!("Should error: Too many heartbeat tasks active");
    };

    let nss = gpio::Output::new(p.PIN_3, gpio::Level::High);
    let reset = gpio::Output::new(p.PIN_15, gpio::Level::High);
    let dio1 = gpio::Input::new(p.PIN_20, gpio::Pull::None);

    let spi = spi::Spi::new(
        p.SPI1,
        p.PIN_10,
        p.PIN_11,
        p.PIN_12,
        p.DMA_CH0,
        p.DMA_CH1,
        spi::Config::default(),
    );

    let lora_spi = match ExclusiveDevice::new(spi, nss, Delay) {
        Ok(lora_spi) => lora_spi,
        Err(e) => {
            log::error!("Failed to create LoRa spi device: {:?}", e);
            loop {}
        }
    };

    let config = sx127x::Config {
        chip: Sx1276,
        tcxo_used: false,
        tx_boost: true,
        rx_boost: true,
    };

    let iv = match lora_phy::iv::GenericSx127xInterfaceVariant::new(reset, dio1, None, None) {
        Ok(iv) => iv,
        Err(e) => {
            log::error!("Failed to create LoRa Interface: {:?}", e);
            loop {}
        }
    };

    let lora = match LoRa::new(Sx127x::new(lora_spi, iv, config), false, Delay).await {
        Ok(l) => l,
        Err(e) => {
            log::error!("Failed to create lora obj: {:?}", e);
            loop {}
        }
    };

    if let Err(e) = spawner.spawn(lora_rcvr(lora)) {
        log::error!("Failed to spawn lora task: {:?}", e);
        loop {}
    };

    loop {
        {
            let mut led_unlocked = LED.lock().await;
            if let Some(pin_ref) = led_unlocked.as_mut() {
                pin_ref.toggle();
                log::info!("led toggled!");
            }
        }
        Timer::after_secs(1).await;
    }
}
