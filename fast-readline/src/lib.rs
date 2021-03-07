use aiofut::AIOBuilder;
use std::fs;
use std::os::unix::io::{AsRawFd, RawFd};
use std::sync::atomic::{AtomicU32, Ordering};

pub struct LineReader {
    buf_tot_byte: u64,
    concurrency: u32,
    shard_num: u32,
    shard_buf_byte: u32,
    file_name: String,
    shards: Vec<ReadShard>,
    read_file: fs::File,
    file_fd: RawFd,
    aio_ctx: aiofut::AIOManager,
    cur_shard_id: AtomicU32,
}

impl LineReader {
    // the params should be divided each other, better to be 2^k
    // TODO handle error
    pub fn new(buf_tot_byte: u64, concurrency: u32, shard_num: u32, file_name: String) -> Self {
        if buf_tot_byte % concurrency as u64 != 0 || concurrency % shard_num != 0 {
            panic!("{} {} {}", buf_tot_byte, concurrency, shard_num);
        }
        let shard_buf_byte = buf_tot_byte / concurrency as u64;
        let shards: Vec<ReadShard> = Vec::with_capacity(shard_num as usize);
        let aio_ctx = AIOBuilder::default().build().unwrap();
        let file = fs::OpenOptions::new()
            .read(true)
            .open(file_name.clone())
            .unwrap();
        let fd = file.as_raw_fd();
        return Self {
            buf_tot_byte: buf_tot_byte,
            concurrency: concurrency,
            shard_num: shard_num,
            shard_buf_byte: shard_buf_byte as u32,
            file_name: file_name,
            shards: shards,
            read_file: file,
            file_fd: fd,
            aio_ctx: aio_ctx,
            cur_shard_id: AtomicU32::new(0),
        };
    }

    fn nextShardId(&mut self) -> u32 {
        return self.cur_shard_id.fetch_add(1, Ordering::SeqCst);
    }
    fn ShardOffset(&mut self, id: u32) -> u64 {
        return id as u64 * self.shard_buf_byte as u64;
    }
}

impl Drop for LineReader {
    fn drop(&mut self) {}
}

pub struct ReadShard {
    first_str: Vec<u8>,
    last_str: Vec<u8>,
}

pub struct ShardReader {
    buf: Vec<u8>,
    shard_id: u32,
}

#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {
        assert_eq!(2 + 2, 4);
    }
}
