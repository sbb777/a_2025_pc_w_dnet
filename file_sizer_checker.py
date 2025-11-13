import os

def find_large_files(directory, size_limit_mb=100):
    """
    Recursively find files larger than the given size limit in the specified directory.

    Args:
        directory (str): Path to the directory to search.
        size_limit_mb (int): File size limit in megabytes (default is 100MB).

    Returns:
        list: A list of tuples containing the file path and its size in megabytes.
    """
    large_files = []
    size_limit_bytes = size_limit_mb * 1024 * 1024

    for root, _, files in os.walk(directory):
        for file in files:
            file_path = os.path.join(root, file)
            try:
                file_size = os.path.getsize(file_path)
                if file_size > size_limit_bytes:
                    large_files.append((file_path, file_size / (1024 * 1024)))
            except OSError as e:
                print(f"Error accessing file {file_path}: {e}")

    return large_files

# Example usage:
if __name__ == "__main__":
    # directory_to_check = input("Enter the directory path to check: ").strip()
    # directory_to_check = r'D:\git\iot_work'
    # directory_to_check = r'D:\git\a_pcbs'
    directory_to_check = r'D:\git\STM32'
    directory_to_check = r'D:\git\a_pcb'
    directory_to_check = r'D:\git\dgv_sw_test'
    directory_to_check = r'D:\git\netx_90_f429_SPI5'
    directory_to_check = r'D:\git\a_2025_pc_w_dnet'

    size_limit_mb = 100  # GitHub's file size limit is 100MB by default
    size_limit_mb = 99  # GitHub's file size limit is 100MB by default

    large_files = find_large_files(directory_to_check, size_limit_mb)

    if large_files:
        print(f"Files larger than {size_limit_mb}MB:")
        for file_path, file_size in large_files:
            print(f"{file_path}: {file_size:.2f} MB")
    else:
        print(f"No files larger than {size_limit_mb}MB found in {directory_to_check}.")