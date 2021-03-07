use crate::{
    config::Config,
    hash,
};

pub struct Spliter {
    out_prefix: string,
    depth: u32,
    config: Arc<Config>,

};
