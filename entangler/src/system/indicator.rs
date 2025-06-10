use defmt::*;
use embassy_rp::gpio::{Output, Level};
use embassy_time::Timer;
use {defmt_rtt as _, panic_probe as _};

use crate::{event, resources::IndicatorResources};

#[embassy_executor::task]
pub async fn indicator_task(r: IndicatorResources) {
    let mut led = Output::new(r.led_pin, Level::Low);
    let mut _event_sub = event::EVENT_CHANNEL.subscriber().unwrap();

    loop {
        debug!("led on!");
        led.set_high();
        Timer::after_secs(1).await;

        debug!("led off!");
        led.set_low();
        Timer::after_secs(1).await;
    }
}
