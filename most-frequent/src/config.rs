#[derive(Clone, Debug)]
pub struct Config {
    pub mem_size_byte: u64,
    pub buf_per_reader_byte: u64,
    pub split_num: u32,
    pub max_split_depth: u32,
    pub top_num: u32,
    pub input_file: String,
    pub output_file: String,
//XXX(st1page): if could be better?
    // pub max_reader_num,
    // pub min_reader_buf_byte,
}

impl Default for Config {
    fn default() -> Self {
        Self {
            mem_size_byte: 1048576000,
            buf_per_reader_byte: 40960000,
            split_num: 1000,
            max_split_depth: 2,
            top_num: 100,
            input_file: "url.txt".to_string(),
            output_file: "result.txt".to_string(),
        }
    }
}
