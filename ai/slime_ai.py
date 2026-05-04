import torch


def main():
    print("Slime AI Python environment is ready.")
    print(f"PyTorch version: {torch.__version__}")
    print(f"CUDA available: {torch.cuda.is_available()}")

    test_tensor = torch.tensor([1.0, 2.0, 3.0])
    print(f"Test tensor: {test_tensor}")


if __name__ == "__main__":
    main()
