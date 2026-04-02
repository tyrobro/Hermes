import random
import time

NUM_TICKS = 10_000_000
FILENAME = "massive_data.csv"

print(f"Generating {NUM_TICKS:,} market ticks...")
start_time = time.time()

with open(FILENAME, "w") as f:
    timestamp = 1625097600000000
    price_base = 150.00

    lines = []

    for i in range(NUM_TICKS):
        timestamp += random.randint(1, 50)
        instrument_id = random.choice([0, 1])

        if random.random() > 0.5:
            price_base += 0.01
        else:
            price_base -= 0.01

        volume = random.randint(1, 20) * 100
        side = random.choice([0, 1])

        lines.append(f"{timestamp},{instrument_id},{price_base:.2f},{volume},{side}\n")

        if len(lines) >= 100_000:
            f.writelines(lines)
            lines.clear()

    if lines:
        f.writelines(lines)

print(f"Done in {time.time() - start_time:.2f} seconds.")
print(f"File saved as {FILENAME}")
