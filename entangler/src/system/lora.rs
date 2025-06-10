use defmt::*;
use embassy_executor::Spawner;
use embassy_rp::gpio::{Input, Level, Output, Pin, Pull};
use embassy_rp::spi::{Config, Spi};
use embassy_time::{Delay, Timer};
use embedded_hal_bus::spi::ExclusiveDevice;
use lora_phy::iv::GenericSx126xInterfaceVariant;
use lora_phy::sx126x::{Sx1262, Sx126x, TcxoCtrlVoltage};
use lora_phy::{mod_params::*, sx126x};
use lora_phy::{LoRa, RxMode};
use {defmt_rtt as _, panic_probe as _};

const LORA_FREQUENCY_IN_HZ: u32 = 915_000_000; // warning: set this appropriately for the region

#[embassy_executor::task]
async fn lora_task(r: LoraResources) {
    let nss = Output::new(r.cs_pin.degrade(), Level::High);
    let reset = Output::new(r.reset_pin.degrade(), Level::High);
    // let dio1 = Input::new(r.dio0.degrade(), Pull::None);
    // let busy = Input::new(r.dio0.degrade(), Pull::None);

    let spi = Spi::new(
        r.spi,
        r.mosi_pin,
        r.miso_pin,
        r.clk_pin,
        r.rx_dma,
        r.tx_dma,
        Config::default(),
    );
    let spi = ExclusiveDevice::new(spi, nss, Delay).unwrap();

    let config = sx126x::Config {
        chip: Sx1262,
        tcxo_ctrl: Some(TcxoCtrlVoltage::Ctrl1V7),
        use_dcdc: true,
        rx_boost: false,
    };
    let iv = GenericSx126xInterfaceVariant::new(reset, None, None, None, None).unwrap();
    let mut lora = LoRa::new(Sx126x::new(spi, iv, config), true, Delay)
        .await
        .unwrap();

    let mut debug_indicator = Output::new(p.PIN_25, Level::Low);

    let mut receiving_buffer = [00u8; 100];

    let mdltn_params = {
        match lora.create_modulation_params(
            SpreadingFactor::_7,
            Bandwidth::_250KHz,
            CodingRate::_4_8,
            LORA_FREQUENCY_IN_HZ,
        ) {
            Ok(mp) => mp,
            Err(err) => {
                info!("Radio error = {}", err);
                return;
            }
        }
    };

    let rx_pkt_params = {
        match lora.create_rx_packet_params(
            4,
            false,
            receiving_buffer.len() as u8,
            true,
            false,
            &mdltn_params,
        ) {
            Ok(pp) => pp,
            Err(err) => {
                info!("Radio error = {}", err);
                return;
            }
        }
    };

    match lora
        .prepare_for_rx(RxMode::Continuous, &mdltn_params, &rx_pkt_params)
        .await
    {
        Ok(()) => {}
        Err(err) => {
            info!("Radio error = {}", err);
            return;
        }
    };

    loop {
        receiving_buffer = [00u8; 100];
        match lora.rx(&rx_pkt_params, &mut receiving_buffer).await {
            Ok((received_len, _rx_pkt_status)) => {
                if (received_len == 3)
                    && (receiving_buffer[0] == 0x01u8)
                    && (receiving_buffer[1] == 0x02u8)
                    && (receiving_buffer[2] == 0x03u8)
                {
                    info!("rx successful");
                    debug_indicator.set_high();
                    Timer::after_secs(5).await;
                    debug_indicator.set_low();
                } else {
                    info!("rx unknown packet");
                }
            }
            Err(err) => info!("rx unsuccessful = {}", err),
        }
    }
}

