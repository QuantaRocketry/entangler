use crate::PacketTrait;

use serde::{Deserialize, Serialize};
#[derive(Serialize, Deserialize, Debug, Eq, PartialEq)]
pub struct GNSS {
    pub latitude: u32,
    pub longitude: u32,
}

impl PacketTrait for GNSS {
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
        let output: Vec<u8, 11> = to_vec(&GNSS {
            latitude: 132,
            longitude: 2,
        })
        .unwrap();

        assert_eq!(&[132, 1, 2,], output.deref());

        let out: GNSS = from_bytes(output.deref()).unwrap();
        assert_eq!(
            out,
            GNSS {
                latitude: 132,
                longitude: 2
            }
        );
    }
}