use embassy_executor::{SpawnError, Spawner};
use embassy_rp::peripherals::USB;
use embassy_rp::usb::Driver;
use embassy_usb::class::cdc_acm::{self, CdcAcmClass};
use embassy_usb::class::cdc_ncm::{self, CdcNcmClass};
use embassy_usb::UsbDevice;
use static_cell::StaticCell;
use {defmt_rtt as _, panic_probe as _};

use crate::{
    event::{self, interface::InterfaceEvent, SystemEvent},
    resources::{InterfaceResources, Irqs},
};

pub mod acm;
pub mod ncm;

pub async fn start(spawner: &Spawner, r: InterfaceResources) -> Result<(), SpawnError> {
    // Create the driver, from the HAL.
    let driver = Driver::new(r.usb, Irqs);

    let mut config = embassy_usb::Config::new(0xc0de, 0xcafe);
    config.manufacturer = Some("Embassy");
    config.product = Some("USB-serial example");
    config.serial_number = Some("12345678");
    config.max_power = 100;
    config.max_packet_size_0 = 64;

    static CONFIG_DESCRIPTOR: StaticCell<[u8; 256]> = StaticCell::new();
    static BOS_DESCRIPTOR: StaticCell<[u8; 256]> = StaticCell::new();
    static CONTROL_BUF: StaticCell<[u8; 64]> = StaticCell::new();

    let mut builder = embassy_usb::Builder::new(
        driver,
        config,
        CONFIG_DESCRIPTOR.init([0; 256]),
        BOS_DESCRIPTOR.init([0; 256]),
        &mut [], // no msos descriptors
        CONTROL_BUF.init([0; 64]),
    );

    // Create classes on the builder.
    let acm_class = {
        static STATE: StaticCell<cdc_acm::State> = StaticCell::new();
        let state = STATE.init(cdc_acm::State::new());
        CdcAcmClass::new(&mut builder, state, 64)
    };

    // Our MAC addr.
    // Host's MAC addr. This is the MAC the host "thinks" its USB-to-ethernet adapter has.
    let host_mac_addr = [0x88, 0x88, 0x88, 0x88, 0x88, 0x88];

    let ncm_class = {
        static STATE: StaticCell<cdc_ncm::State> = StaticCell::new();
        let state = STATE.init(cdc_ncm::State::new());
        CdcNcmClass::new(&mut builder, state, host_mac_addr, 64)
    };

    let usb = builder.build();

    if let Err(err) = spawner.spawn(usb_task(usb)) {
        let e = SystemEvent::Interface(InterfaceEvent::UsbSpawnError);
        if let Ok(publisher) = event::EVENT_CHANNEL.publisher() {
            publisher.publish(e).await;
        };
        return Err(err);
    };

    let acm_res = acm::start(spawner, acm_class);
    
    // let ncm_res = ncm::start(spawner, ncm_class);
    let ncm_res = Ok(());

    if let Err(_) = acm_res {
        let e = SystemEvent::Interface(InterfaceEvent::AcmSpawnError);
        if let Ok(publisher) = event::EVENT_CHANNEL.publisher() {
            publisher.publish(e).await;
        };
    };

    if let Err(_) = ncm_res {
        let e = SystemEvent::Interface(InterfaceEvent::NcmSpawnError);
        if let Ok(publisher) = event::EVENT_CHANNEL.publisher() {
            publisher.publish(e).await;
        };
    };

    acm_res?;
    ncm_res?;
    Ok(())
}

#[embassy_executor::task]
async fn usb_task(mut usb: UsbDevice<'static, Driver<'static, USB>>) -> ! {
    usb.run().await
}
