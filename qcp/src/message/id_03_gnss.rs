use super::MessageTrait;

use zerocopy::*;

#[derive(Debug, Eq, PartialEq, Copy, Clone, FromBytes, IntoBytes, KnownLayout, Immutable)]
#[repr(C)]
pub struct GNSS {
    pub latitude: u32,
    pub longitude: u32,
}

impl MessageTrait for GNSS {
    const PACKET_ID: u8 = 3;
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn direct() {
        let output: &[u8] = GNSS {
            latitude: 132,
            longitude: 2,
        }
        .as_bytes();

        assert_eq!(&[132, 0, 0, 0, 2, 0, 0, 0], output);

        let out = GNSS::ref_from_bytes(output).unwrap();
        assert_eq!(
            out,
            &GNSS {
                latitude: 132,
                longitude: 2
            }
        );
    }
}
