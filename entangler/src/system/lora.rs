use defmt::*;
use embassy_executor::Spawner;
use embassy_rp::gpio::{Input, Level, Output, Pin, Pull};
use embassy_rp::spi::{Config, Spi};
use embassy_time::{Delay, Timer};
use embedded_hal_bus::spi::ExclusiveDevice;
use lora_phy::iv::GenericSx127xInterfaceVariant;
use lora_phy::sx127x::{Sx1276, Sx127x};
use lora_phy::{mod_params::*, sx127x};
use lora_phy::{LoRa, RxMode};
use {defmt_rtt as _, panic_probe as _};

use crate::{event, resources::LoraResources};

const LORA_FREQUENCY_IN_HZ: u32 = 915_000_000; // warning: set this appropriately for the region
const LORA_TX_PWR: i32 = 20;

#[embassy_executor::task]
pub async fn lora_task(r: LoraResources) {
    let nss = Output::new(r.cs_pin.degrade(), Level::High);
    let reset = Output::new(r.reset_pin.degrade(), Level::High);
    let irq = Input::new(r.dio0_pin.degrade(), Pull::None);

    let spi = Spi::new(
        r.spi,
        r.clk_pin,
        r.mosi_pin,
        r.miso_pin,
        r.tx_dma,
        r.rx_dma,
        Config::default(),
    );

    let spi = ExclusiveDevice::new(spi, nss, Delay).unwrap();

    let config = sx127x::Config {
        chip: Sx1276,
        tcxo_used: false,
        tx_boost: true,
        rx_boost: false,
    };
    let iv = GenericSx127xInterfaceVariant::new(reset, irq, None, None).unwrap();
    let mut lora = LoRa::new(Sx127x::new(spi, iv, config), true, Delay)
        .await
        .unwrap();

    let mdltn_params = {
        match lora.create_modulation_params(
            SpreadingFactor::_10,
            Bandwidth::_250KHz,
            CodingRate::_4_8,
            LORA_FREQUENCY_IN_HZ,
        ) {
            Ok(mp) => mp,
            Err(err) => {
                error!("Radio error = {}", err);
                return;
            }
        }
    };

    let buffer = [b'e', b'n', b't', b'a', b'n', b'g', b'l', b'e', b'r'];

    loop {
        Timer::after_secs(30).await;

        let mut tx_pkt_params = {
            match lora.create_tx_packet_params(4, false, true, false, &mdltn_params) {
                Ok(pp) => pp,
                Err(err) => {
                    info!("Radio error = {}", err);
                    return;
                }
            }
        };

        match lora
            .prepare_for_tx(&mdltn_params, &mut tx_pkt_params, LORA_TX_PWR, &buffer)
            .await
        {
            Ok(()) => {}
            Err(err) => {
                info!("Radio error = {}", err);
                continue;
            }
        };

        match lora.tx().await {
            Ok(()) => {
                info!("TX DONE");
            }
            Err(err) => {
                error!("Radio error = {}", err);
                continue;
            }
        };
    }
}
