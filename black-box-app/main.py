from ui.main_window import launch_app as launch_gui
from ui.cli import launch_app as launch_cli

if __name__ == "__main__":
    choice = input("Choose interface (GUI/CLI): ").strip().lower()
    if choice == "gui":
        launch_gui()
    elif choice == "cli":
        launch_cli()
    else:
        print("Invalid choice. Please choose 'GUI' or 'CLI'.")
