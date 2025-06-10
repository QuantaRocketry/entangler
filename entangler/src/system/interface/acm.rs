use defmt::*;
use embassy_executor::{SpawnError, Spawner};
use embassy_rp::peripherals::USB;
use embassy_rp::usb::Driver;
use embassy_usb::class::cdc_acm::CdcAcmClass;
use {defmt_rtt as _, panic_probe as _};

use crate::event::SystemEvent;
use crate::event::{self, interface::InterfaceEvent};

pub fn start(
    spawner: &Spawner,
    class: CdcAcmClass<'static, Driver<'static, USB>>,
) -> Result<(), SpawnError> {
    spawner.spawn(acm_task(class))?;
    Ok(())
}

#[embassy_executor::task]
async fn acm_task(mut class: CdcAcmClass<'static, Driver<'static, USB>>) -> ! {
    let mut buf = [0; 64];
    let event_publisher = unwrap!(event::EVENT_CHANNEL.publisher());

    loop {
        info!("Waiting for connection...");
        class.wait_connection().await;

        info!("Connected");
        event_publisher.publish(SystemEvent::Interface(InterfaceEvent::Connected)).await;

        loop {
            let n = match class.read_packet(&mut buf).await {
                Ok(n) => n,
                Err(e) => {
                    let event = event::SystemEvent::Interface(InterfaceEvent::from(e));
                    event_publisher.publish(event).await;
                    break;
                }
            };
            let data = &buf[..n];
            info!("data: {:x}", data);
            if let Err(e) = class.write_packet(data).await {
                let e = event::SystemEvent::Interface(InterfaceEvent::from(e));
                event_publisher.publish(e).await;
                break;
            };
        }
        info!("Disconnected");
    }
}
