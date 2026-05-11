import random
from pathlib import Path

def generate_spawns_txt(count, x_min, x_max, y_min, y_max):
    output_dir = Path("extra")
    output_dir.mkdir(parents=True, exist_ok=True)
    
    file_path = output_dir / "spawn_points.txt"
    
    lines = ["const SDL_FPoint spawnPoints[] = {"]
    for i in range(count):
        x = random.randint(x_min, x_max)
        y = random.randint(y_min, y_max)
        comma = "," if i < count - 1 else ""
        lines.append(f"    SDL_FPoint{{{x}.0f, {y}.0f}}{comma}")
    lines.append("};")

    with open(file_path, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")

    print(f"Файл сохранён: {file_path}")
    print(f"Сгенерировано точек: {count}")
    print(f"Диапазон: X[{x_min}-{x_max}], Y[{y_min}-{y_max}]")

if __name__ == "__main__":
    generate_spawns_txt(count=1000, x_min=0, x_max=1920, y_min=0, y_max=1080)