use std::path::Path;

pub fn load_binary(file_name: &str) -> anyhow::Result<Vec<u8>> {
    let path = std::path::Path::new(env!("OUT_DIR"))
        .join("res")
        .join(file_name);
    println!("Loading binary at {:?}...", path.to_str().unwrap());
    let data = std::fs::read(path)?;
    Ok(data)
}

pub fn load_string(file_name: &str) -> anyhow::Result<String> {
    let path = std::path::Path::new(env!("OUT_DIR"))
        .join("res")
        .join(file_name);
    println!("Loading string at {:?}...", path.to_str().unwrap());
    let txt = std::fs::read_to_string(path)?;
    Ok(txt)
}

pub fn load_string_from_path(path: &Path) -> anyhow::Result<String> {
    let path = Path::new(env!("OUT_DIR")).join("res").join(path);
    println!("Loading string from path {:?}...", path.to_str().unwrap());
    let txt = std::fs::read_to_string(path).unwrap();
    Ok(txt)
}
