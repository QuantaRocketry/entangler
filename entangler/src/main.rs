//! This example test the RP Pico on board LED.
//!
//! It does not work with the RP Pico W board. See wifi_blinky.rs.

#![no_std]
#![no_main]

use defmt::*;
use embassy_executor::Spawner;
use {defmt_rtt as _, panic_probe as _};

use crate::event::{indicator::IndicatorEvent, SystemEvent};
use crate::resources::{AssignedResources, IndicatorResources, InterfaceResources, LoraResources};
use crate::system::{indicator::indicator_task, interface};

mod event;
mod resources;
mod state;
mod system;

#[embassy_executor::main]
async fn main(spawner: Spawner) {
    let p = embassy_rp::init(Default::default());
    let r = split_resources!(p);

    if let Err(_) = spawner.spawn(indicator_task(r.indicators)) {
        let e = SystemEvent::Indicator(IndicatorEvent::SpawnError);
        if let Ok(publisher) = event::EVENT_CHANNEL.publisher() {
            publisher.publish(e).await;
        };
    };
    let _ = interface::start(&spawner, r.interface).await;

    info!("All tasks spawned");
}
