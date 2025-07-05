use crate::PacketTrait;

use serde::{Deserialize, Serialize};
#[derive(Serialize, Deserialize, Debug, Eq, PartialEq)]
pub struct Heartbeat {
    pub uptime: u32,
}

impl PacketTrait for Heartbeat {
    const PACKET_ID: u32 = 1;
}

#[cfg(test)]
mod tests {
    use super::*;
    use core::ops::Deref;
    use heapless::Vec;
    use postcard::{from_bytes, to_vec};

    #[test]
    fn ref_struct() {
        let output: Vec<u8, 11> = to_vec(&Heartbeat { uptime: 13298326 }).unwrap();

        assert_eq!(&[150, 213, 171, 6], output.deref());

        let out: Heartbeat = from_bytes(output.deref()).unwrap();
        assert_eq!(out, Heartbeat { uptime: 13298326 });
    }
}
