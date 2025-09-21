#!/usr/bin/env python3
"""
Cross-platform setup script for Article Scraper project
Works on Windows, Linux, and macOS - No Virtual Environment Version
"""

import os
import sys
import subprocess
import platform
from pathlib import Path

def run_command(cmd, cwd=None, shell=None):
    """Run command with proper shell setting for each platform"""
    if shell is None:
        shell = platform.system() == "Windows"
    
    try:
        result = subprocess.run(cmd, cwd=cwd, shell=shell, check=True, 
                              capture_output=True, text=True)
        return True, result.stdout
    except subprocess.CalledProcessError as e:
        return False, e.stderr

def check_cmake():
    """Check if CMake is installed"""
    success, output = run_command(["cmake", "--version"])
    if success:
        print(f"CMake found: {output.split()[2] if len(output.split()) > 2 else 'version unknown'}")
        return True
    else:
        print("CMake not found. Please install CMake and add it to your PATH")
        return False

def main():
    print("Setting up Article Scraper project...")
    
    # Get the directory where this script is located
    script_dir = Path(__file__).parent.absolute()
    os.chdir(script_dir)
    print(f"Working in directory: {script_dir}")
    
    # Check if CMake is available
    if not check_cmake():
        print("Cannot proceed without CMake. Please install it first.")
        return
    
    # Check vcpkg
    vcpkg_root = os.environ.get('VCPKG_ROOT')
    if not vcpkg_root:
        print("Warning: VCPKG_ROOT environment variable not set")
        print("Make sure vcpkg is installed and VCPKG_ROOT points to its directory")
    
    # Create project structure
    print("Creating project structure...")
    Path("flask-api").mkdir(exist_ok=True)
    Path("cpp-script/build").mkdir(parents=True, exist_ok=True)
    
    # Check for CMakeLists.txt
    cmake_path = Path("cpp-script/CMakeLists.txt")
    if not cmake_path.exists():
        print("CMakeLists.txt not found in cpp-script directory")
        print(f"Looking for: {cmake_path.absolute()}")
        print("Please ensure the CMakeLists.txt is in the cpp-script folder")
        return
    else:
        print(f"Found CMakeLists.txt at: {cmake_path.absolute()}")
    
    # Install vcpkg dependencies
    if vcpkg_root:
        print("Installing vcpkg dependencies...")
        vcpkg_exe = "vcpkg.exe" if platform.system() == "Windows" else "vcpkg"
        vcpkg_path = Path(vcpkg_root) / vcpkg_exe
        
        if vcpkg_path.exists():
            cmd = [str(vcpkg_path), "install", "lexbor", "cpr", "nlohmann-json", "tbb"]
            success, output = run_command(cmd)
            if success:
                print("vcpkg dependencies installed successfully")
            else:
                print(f"Failed to install vcpkg dependencies: {output}")
                print("Continuing anyway - dependencies might already be installed")
        else:
            print(f"vcpkg executable not found at {vcpkg_path}")
    
    # Build C++ project
    print("\nBuilding C++ project...")
    cpp_script_dir = Path("cpp-script")
    build_dir = cpp_script_dir / "build"
    
    # Configure with CMake
    print("Configuring with CMake...")
    cmake_cmd = ["cmake", "..", "-DCMAKE_BUILD_TYPE=Release"]
    
    # Add vcpkg toolchain if available
    if vcpkg_root:
        toolchain_file = Path(vcpkg_root) / "scripts" / "buildsystems" / "vcpkg.cmake"
        if toolchain_file.exists():
            cmake_cmd.extend([f"-DCMAKE_TOOLCHAIN_FILE={toolchain_file}"])
            print(f"Using vcpkg toolchain: {toolchain_file}")
    
    success, output = run_command(cmake_cmd, cwd=build_dir)
    if success:
        print("CMake configuration successful")
        if output.strip():
            print("Configuration output:")
            print(output)
    else:
        print(f"CMake configuration failed: {output}")
        print("\nTroubleshooting tips:")
        print("1. Make sure all vcpkg dependencies are installed")
        print("2. Check that VCPKG_ROOT is set correctly")
        print("3. Ensure pkg-config is installed (Linux/macOS)")
        return
    
    # Build the project
    print("Building the project...")
    build_cmd = ["cmake", "--build", ".", "--config", "Release"]
    success, output = run_command(build_cmd, cwd=build_dir)
    if success:
        print("C++ project built successfully!")
        if output.strip():
            print("Build output:")
            print(output)
        
        # Check if executable was created
        exe_name = "article_scraper.exe" if platform.system() == "Windows" else "article_scraper"
        exe_path = build_dir / exe_name
        if exe_path.exists():
            print(f"Executable created: {exe_path.absolute()}")
        else:
            print(f"Warning: Expected executable not found at {exe_path.absolute()}")
            # List files in build directory
            print("Files in build directory:")
            for file in build_dir.iterdir():
                print(f"  {file.name}")
    else:
        print(f"Build failed: {output}")
        print("\nCheck the error messages above for specific issues")
        return
    
    # Install Python dependencies globally
    print("\nInstalling Python dependencies...")
    flask_api_dir = Path("flask-api")
    
    requirements_path = flask_api_dir / "requirements.txt"
    if requirements_path.exists():
        print(f"Found requirements.txt at: {requirements_path.absolute()}")
        print("Installing packages globally with pip...")
        
        # Use system pip to install globally
        success, output = run_command([sys.executable, "-m", "pip", "install", "-r", str(requirements_path)])
        if success:
            print("Python dependencies installed successfully")
            if output.strip():
                print("Installation output:")
                print(output)
        else:
            print(f"Failed to install Python dependencies:")
            print(f"Error: {output}")
            
            # Try with --user flag as fallback
            print("Trying with --user flag...")
            success, output = run_command([sys.executable, "-m", "pip", "install", "--user", "-r", str(requirements_path)])
            if success:
                print("Python dependencies installed successfully with --user flag")
            else:
                print(f"Still failed: {output}")
    else:
        print(f"requirements.txt not found at: {requirements_path.absolute()}")
        print("Skipping Python dependency installation")
    
    print("\n" + "="*50)
    print("Setup complete!")
    print("="*50)
    print("To run the project:")
    print("1. Make sure MongoDB is running")
    print("2. Run the Flask app: cd flask-api && python app.py")
    print(f"3. C++ executable is available at: cpp-script/build/")
    print("\nNote: Dependencies are installed globally (no virtual environment)")

if __name__ == "__main__":
    main()