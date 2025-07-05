use std::usize;

pub mod error;
mod header;

pub mod message;
pub use message::Message;

const PACKET_SIZE_MAX: usize = 32;
