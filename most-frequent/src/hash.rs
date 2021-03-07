mod hash{
    const seeds_num = 31
    const seeds: [i32; seeds_num] = [131, 137, 139, 149, 151, 157, 163, 167,
                                    173, 179, 181, 191, 193, 197, 199, 211,
                                    223, 227, 229, 233, 239, 241, 251, 257,
                                    263, 269, 271, 277, 281, 283, 293]; 
    pub fn hash(buf: &[u8], u64) -> u64 {
        let mut ret:u64 = 0
        for c in buf {
            ret = ret * seed + c
        }
        retrun ret
    }
                                    
}
