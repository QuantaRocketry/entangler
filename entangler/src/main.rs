//! This example test the RP Pico on board LED.
//!
//! It does not work with the RP Pico W board. See wifi_blinky.rs.

#![no_std]
#![no_main]

use defmt::*;
use embassy_executor::Spawner;
use {defmt_rtt as _, panic_probe as _};

use crate::system::resources::{AssignedResources, IndicatorResources, LoraResources};

use crate::task::indicator::indicator_task;
mod system;
mod task;

#[embassy_executor::main]
async fn main(spawner: Spawner) {
    let p = embassy_rp::init(Default::default());
    let r = split_resources!(p);

    spawner.spawn(indicator_task(r.indicators)).unwrap();

    info!("All tasks spawned");
}
