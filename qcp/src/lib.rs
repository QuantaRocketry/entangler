pub mod event;
pub mod request;
pub mod state;

use std::usize;

use heapless::Vec;
use postcard::to_vec;
use serde::{Deserialize, Serialize};

const PACKET_SIZE_MAX: usize = 32;

pub trait PacketTrait: Serialize + for<'de> Deserialize<'de> {
    const PACKET_ID: u32;
}

pub fn serialize<T: PacketTrait>(packet: &T) -> postcard::Result<Vec<u8, PACKET_SIZE_MAX>> {
    let mut header = match Vec::from_slice(&T::PACKET_ID.to_be_bytes()) {
        Ok(h) => h,
        Err(_) => return Err(postcard::Error::NotYetImplemented),
    };

    let data: Vec<u8, PACKET_SIZE_MAX> = to_vec(packet)?;

    header.extend_from_slice(data.());

    Ok(header)
}

pub fn deserialize<T: PacketTrait>(data: &[u8]) -> postcard::Result<T> {
    let packet_id: [u8; 4] = data[0..4].try_into().unwrap();
    let packet_id = u32::from_be_bytes(packet_id);

    Err(postcard::Error::NotYetImplemented)
}
