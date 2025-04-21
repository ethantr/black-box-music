# Check if python is installed on MAC
if ! command -v python &> /dev/null
then
    echo "Python is not installed. Please install Python 3."
    exit
fi

# Check if pip is installed
if ! command -v pip &> /dev/null
then
    echo "pip is not installed. Please install pip."
    exit
fi

# Create a virtual environment
if [ ! -d "venv" ]; then
    echo "Creating virtual environment..."
    python -m venv venv
fi
# Activate the virtual environment
source venv/bin/activate
# Install the required packages
pip install -r requirements.txt
# Check if the installation was successful
if [ $? -eq 0 ]; then
    echo "Installation successful!"
else
    echo "Installation failed."
    exit 1
fi