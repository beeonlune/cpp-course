from pathlib import Path

import pandas as pd
import matplotlib.pyplot as plt


script_dir = Path(__file__).resolve().parent
csv_path = script_dir / "hash_collisions.csv"

data = pd.read_csv(csv_path)

data["N_thousands"] = data["N"] / 1000

data["Expected uniform 32-bit"] = data["N"] * (data["N"] - 1) / (2 * (2 ** 32))

plt.figure(figsize=(14, 8))

for column in data.columns:
    if column not in {"N", "N_thousands", "Expected uniform 32-bit"}:
        if column == "PJW Hash":
            plt.plot(
                data["N_thousands"],
                data[column],
                marker="o",
                linewidth=2,
                linestyle="--",
                label=column
            )
        elif column == "ELF Hash":
            plt.plot(
                data["N_thousands"],
                data[column],
                marker="o",
                linewidth=2,
                linestyle="-",
                label=column
            )
        else:
            plt.plot(
                data["N_thousands"],
                data[column],
                marker="o",
                linewidth=2,
                label=column
            )

plt.title("Зависимость количества коллизий от количества строк")
plt.xlabel("Количество хэшируемых строк, тыс.")
plt.ylabel("Количество коллизий")
plt.grid(True, linestyle="--", alpha=0.5)
plt.legend()
plt.tight_layout()
plt.savefig(script_dir / "hash_collisions_all.png", dpi=300)
plt.show()


# graph 2: normal hash functions + theoretical expected line
columns_to_skip = {"N", "N_thousands", "PJW Hash", "ELF Hash"}

plt.figure(figsize=(14, 8))

for column in data.columns:
    if column not in columns_to_skip:
        if column == "Expected uniform 32-bit":
            plt.plot(
                data["N_thousands"],
                data[column],
                linewidth=3,
                linestyle="--",
                label="Ожидаемое значение для равномерного 32-битного хэша"
            )
        else:
            plt.plot(
                data["N_thousands"],
                data[column],
                marker="o",
                linewidth=2,
                label=column
            )

plt.title("Сравнение хэш-функций без PJW и ELF")
plt.xlabel("Количество хэшируемых строк, тыс.")
plt.ylabel("Количество коллизий")
plt.grid(True, linestyle="--", alpha=0.5)
plt.legend()
plt.tight_layout()
plt.savefig(script_dir / "hash_collisions_without_pjw_elf.png", dpi=300)
plt.show()