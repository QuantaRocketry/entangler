use defmt::*;

use super::SystemEvent;

#[derive(Debug, Clone, Format)]
pub enum IndicatorEvent {
    SpawnError,
}

impl From<IndicatorEvent> for SystemEvent {
    fn from(e: IndicatorEvent) -> SystemEvent {
        SystemEvent::Indicator(e)
    }
}
