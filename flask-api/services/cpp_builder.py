import logging
import subprocess
import platform
import os
from pathlib import Path

logger = logging.getLogger(__name__)

class CppBuilderService:
    """Service for building and running C++ executables"""
    
    def __init__(self, config):
        self.config = config
    
    def ensure_executable(self):
        """Build C++ project if executable doesn't exist or is outdated"""
        executable_path = Path(self.config.CPP_EXECUTABLE_PATH)
        project_dir = Path(self.config.CPP_PROJECT_DIR)
        build_dir = Path(self.config.CPP_BUILD_DIR)
        
        # Check if executable exists and is newer than source files
        if executable_path.exists():
            exe_time = executable_path.stat().st_mtime
            source_files = list(project_dir.glob("**/*.cpp")) + list(project_dir.glob("**/*.h"))
            
            if source_files and all(exe_time > f.stat().st_mtime for f in source_files):
                logger.info("C++ executable is up to date")
                return True
        
        logger.info("Building C++ project...")
        return self.build_project()
    
    def build_project(self):
        """Build the C++ project using CMake (cross-platform)"""
        try:
            project_dir = Path(self.config.CPP_PROJECT_DIR)
            build_dir = Path(self.config.CPP_BUILD_DIR)
            
            # Create build directory
            build_dir.mkdir(exist_ok=True)
            
            # Configure with CMake
            if not self._configure_cmake(project_dir, build_dir):
                return False
            
            # Build
            if not self._build_cmake(build_dir):
                return False
            
            logger.info("C++ project built successfully")
            return True
            
        except subprocess.TimeoutExpired:
            logger.error("Build process timed out")
            return False
        except Exception as e:
            logger.error(f"Error building C++ project: {str(e)}")
            return False
    
    def _configure_cmake(self, project_dir, build_dir):
        """Configure CMake build"""
        logger.info("Running CMake configure...")
        configure_cmd = [
            "cmake", 
            str(project_dir),
            f"-DCMAKE_BUILD_TYPE={self.config.CMAKE_BUILD_TYPE}"
        ]
        
        # Add vcpkg toolchain if available
        vcpkg_root = os.environ.get('VCPKG_ROOT')
        if vcpkg_root:
            toolchain_file = Path(vcpkg_root) / "scripts" / "buildsystems" / "vcpkg.cmake"
            configure_cmd.extend([f"-DCMAKE_TOOLCHAIN_FILE={toolchain_file}"])
        
        # Windows-specific generator
        if platform.system() == "Windows":
            configure_cmd.extend(["-G", "Visual Studio 17 2022"])
        
        result = subprocess.run(
            configure_cmd,
            cwd=build_dir,
            capture_output=True,
            text=True,
            encoding="utf-8", 
            errors="replace",
            timeout=300,
            shell=platform.system() == "Windows"
        )
        
        if result.returncode != 0:
            logger.error(f"CMake configure failed: {result.stderr}")
            return False
        
        return True
    
    def _build_cmake(self, build_dir):
        """Build with CMake"""
        logger.info("Building project...")
        build_cmd = ["cmake", "--build", ".", "--config", self.config.CMAKE_BUILD_TYPE]
        
        # Add parallel build flag
        if platform.system() != "Windows":
            build_cmd.extend(["--", "-j4"])
        
        result = subprocess.run(
            build_cmd,
            cwd=build_dir,
            capture_output=True,
            text=True,
            timeout=self.config.BUILD_TIMEOUT,
            shell=platform.system() == "Windows"
        )
        
        if result.returncode != 0:
            logger.error(f"Build failed: {result.stderr}")
            return False
        
        return True
    
    def run_executable(self):
        """Run the C++ script and return the JSON output"""
        # Ensure executable is built and up to date
        if not self.ensure_executable():
            logger.error("Failed to build C++ executable")
            return None
            
        try:
            logger.info("Running C++ script...")
            result = subprocess.run(
                [self.config.CPP_EXECUTABLE_PATH],
                capture_output=True,
                text=True,
                encoding="utf-8",   
                errors="replace",  
                timeout=self.config.SCRIPT_TIMEOUT
            )
            
            if result.returncode != 0:
                logger.error(f"C++ script failed with error: {result.stderr}")
                return None
                
            return result.stdout.strip()
            
        except subprocess.TimeoutExpired:
            logger.error("C++ script timed out")
            return None
        except Exception as e:
            logger.error(f"Error running C++ script: {str(e)}")
            return None