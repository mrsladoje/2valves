import os
import platform
from pathlib import Path

class Config:
    """Base configuration class"""
    
    # Flask settings
    DEBUG = False
    HOST = '0.0.0.0'
    PORT = 5000
    
    # MongoDB configuration
    MONGO_URI = os.environ.get('MONGO_URI', "mongodb://localhost:27017/")
    DATABASE_NAME = os.environ.get('DATABASE_NAME', "articles")
    COLLECTION_NAME = os.environ.get('COLLECTION_NAME', "articles")
    
    # Paths configuration
    PROJECT_ROOT = Path(__file__).resolve().parent.parent
    CPP_PROJECT_DIR = PROJECT_ROOT / "cpp-script"
    CPP_BUILD_DIR = CPP_PROJECT_DIR / "build"
    
    # Cross-platform executable name
    if platform.system() == "Windows":
        CPP_EXECUTABLE_PATH = CPP_BUILD_DIR / "Release" / "article_scraper.exe"
    else:
        CPP_EXECUTABLE_PATH = CPP_BUILD_DIR / "article_scraper"
    
    CPP_EXECUTABLE_PATH = str(CPP_EXECUTABLE_PATH)
    
    # Scheduler settings
    SCRAPER_INTERVAL_HOURS = int(os.environ.get('SCRAPER_INTERVAL_HOURS', 2))
    
    # Build settings
    BUILD_TIMEOUT = 600
    SCRIPT_TIMEOUT = 300
    CMAKE_BUILD_TYPE = "Release"
    
    # Logging
    LOG_LEVEL = os.environ.get('LOG_LEVEL', 'INFO')

class DevelopmentConfig(Config):
    """Development configuration"""
    DEBUG = True

class ProductionConfig(Config):
    """Production configuration"""
    DEBUG = False
    LOG_LEVEL = 'WARNING'

class TestingConfig(Config):
    """Testing configuration"""
    DEBUG = True
    DATABASE_NAME = "articles_test"

# Configuration mapping
config = {
    'development': DevelopmentConfig,
    'production': ProductionConfig,
    'testing': TestingConfig,
    'default': DevelopmentConfig
}