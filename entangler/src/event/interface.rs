use defmt::*;
use embassy_usb::driver::EndpointError;

use super::SystemEvent;

#[derive(Debug, Clone, Format)]
pub enum InterfaceEvent {
    Connected,
    Disconnected,
    BufferOverflow,
    UsbSpawnError,
    AcmSpawnError,
    NcmSpawnError,
}

impl From<EndpointError> for InterfaceEvent {
    fn from(e: EndpointError) -> Self {
        match e {
            EndpointError::BufferOverflow => InterfaceEvent::BufferOverflow,
            EndpointError::Disabled => InterfaceEvent::Disconnected,
        }
    }
}

impl From<InterfaceEvent> for SystemEvent {
    fn from(e: InterfaceEvent) -> SystemEvent {
        SystemEvent::Interface(e)
    }
}
