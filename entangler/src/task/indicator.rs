use defmt::*;
use embassy_rp::gpio::{Level, Output};
use embassy_time::Timer;
use {defmt_rtt as _, panic_probe as _};

use crate::system::resources::IndicatorResources;

#[embassy_executor::task]
pub async fn indicator_task(r: IndicatorResources) {
    let mut led = Output::new(r.led_pin, Level::Low);

    loop {
        info!("led on!");
        led.set_high();
        Timer::after_secs(1).await;

        info!("led off!");
        led.set_low();
        Timer::after_secs(1).await;
    }
}

