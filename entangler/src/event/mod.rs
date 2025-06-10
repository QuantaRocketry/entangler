use defmt::*;
use embassy_executor::SpawnError;
use embassy_sync::{blocking_mutex::raw::CriticalSectionRawMutex, pubsub::PubSubChannel};

pub mod interface;
pub mod indicator;

use interface::InterfaceEvent;
use indicator::IndicatorEvent;
// use crate::system::state
// pub mod settings;

pub static EVENT_CHANNEL: PubSubChannel<CriticalSectionRawMutex, SystemEvent, 10, 10, 10> =
    PubSubChannel::new();

#[derive(Debug, Clone, Format)]
pub enum SystemEvent {
    _Error(SystemError),
    Interface(InterfaceEvent),
    Indicator(IndicatorEvent),
    _StateUpdated,
}

#[derive(Debug, Clone, Format)]
pub enum SystemError {
    _Spawn(SpawnError),
}