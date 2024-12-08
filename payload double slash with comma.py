import random

def generate_random_payloads_to_file():
    # Ask the user for the desired size
    size_input = input("How many KB or MB do you need? (e.g., '512 KB' or '1 MB'): ").strip()
    
    # Split the input into size and unit
    try:
        size_value, unit = size_input.split()
        size_value = int(size_value)  # Convert the size to an integer
        unit = unit.upper()  # Normalize unit to uppercase
    except (ValueError, IndexError):
        print("Invalid input! Please enter a size followed by 'KB' or 'MB'.")
        return

    # Determine the size in bytes
    if unit == 'MB':
        total_bytes = size_value * 1024 * 1024  # Convert MB to bytes
    elif unit == 'KB':
        total_bytes = size_value * 1024  # Convert KB to bytes
    else:
        print("Invalid unit! Please enter either 'KB' or 'MB'.")
        return

    # Define the payload size and calculate the line count
    payload_size = 32  # Size of each payload in bytes
    line_count = total_bytes // payload_size  # Calculate number of payloads needed

    payloads = []
    for _ in range(line_count):
        # Generate a payload without commas between the hex values
        payload = ''.join(f'\\\\x{random.randint(0, 255):02x}' for _ in range(payload_size))
        payloads.append(f'"{payload}",')  # Add comma after each payload

    filename = f'payload by rare {size_value} {unit}.txt'  # Updated filename
    with open(filename, 'w') as f:
        f.write('\n'.join(payloads))  # Join payloads with newlines

    print(f"Payloads saved to {filename}")

# Example usage
generate_random_payloads_to_file()
