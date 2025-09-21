from flask import Flask, jsonify
from pymongo import MongoClient
from apscheduler.schedulers.background import BackgroundScheduler
import subprocess
import json
import logging
import os
import platform
from datetime import datetime
from pathlib import Path

# Configure logging
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

app = Flask(__name__)

# MongoDB configuration
MONGO_URI = "mongodb://localhost:27017/"
DATABASE_NAME = "articles"
COLLECTION_NAME = "articles"

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

class ArticleProcessor:
    def __init__(self):
        self.client = MongoClient(MONGO_URI)
        self.db = self.client[DATABASE_NAME]
        self.collection = self.db[COLLECTION_NAME]
        
    def ensure_cpp_executable(self):
        """Build C++ project if executable doesn't exist or is outdated"""
        executable_path = Path(CPP_EXECUTABLE_PATH)
        project_dir = Path(CPP_PROJECT_DIR)
        build_dir = Path(CPP_BUILD_DIR)
        
        # Check if executable exists and is newer than source files
        if executable_path.exists():
            exe_time = executable_path.stat().st_mtime
            source_files = list(project_dir.glob("**/*.cpp")) + list(project_dir.glob("**/*.h"))
            
            if source_files and all(exe_time > f.stat().st_mtime for f in source_files):
                logger.info("C++ executable is up to date")
                return True
        
        logger.info("Building C++ project...")
        return self.build_cpp_project()
    
    def build_cpp_project(self):
        """Build the C++ project using CMake (cross-platform)"""
        try:
            project_dir = Path(CPP_PROJECT_DIR)
            build_dir = Path(CPP_BUILD_DIR)
            
            # Create build directory
            build_dir.mkdir(exist_ok=True)
            
            # Configure with CMake
            logger.info("Running CMake configure...")
            configure_cmd = [
                "cmake", 
                str(project_dir),
                "-DCMAKE_BUILD_TYPE=Release"
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
            
            # Build
            logger.info("Building project...")
            build_cmd = ["cmake", "--build", ".", "--config", "Release"]
            
            # Add parallel build flag
            if platform.system() != "Windows":
                build_cmd.extend(["--", "-j4"])
            
            result = subprocess.run(
                build_cmd,
                cwd=build_dir,
                capture_output=True,
                text=True,
                timeout=600,
                shell=platform.system() == "Windows"
            )
            
            if result.returncode != 0:
                logger.error(f"Build failed: {result.stderr}")
                return False
            
            logger.info("C++ project built successfully")
            return True
            
        except subprocess.TimeoutExpired:
            logger.error("Build process timed out")
            return False
        except Exception as e:
            logger.error(f"Error building C++ project: {str(e)}")
            return False
        
    def run_cpp_script(self):
        """Run the C++ script and return the JSON output"""
        # Ensure executable is built and up to date
        if not self.ensure_cpp_executable():
            logger.error("Failed to build C++ executable")
            return None
            
        try:
            logger.info("Running C++ script...")
            result = subprocess.run(
                [CPP_EXECUTABLE_PATH],
                capture_output=True,
                text=True,
                encoding="utf-8",   
                errors="replace",  
                timeout=300
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
    
    def save_articles_to_db(self, articles_json):
        """Parse JSON and save articles to MongoDB"""
        try:
            articles = json.loads(articles_json)
            
            # Handle both single article and array of articles
            if isinstance(articles, dict):
                articles = [articles]
            elif not isinstance(articles, list):
                logger.error("Invalid JSON format - expected object or array")
                return False
            
            saved_count = 0
            for article in articles:
                # Add timestamp when saved
                article['saved_at'] = datetime.utcnow()
                
                # Use upsert to avoid duplicates based on _id (URL)
                result = self.collection.update_one(
                    {"_id": article.get("_id")},
                    {"$set": article},
                    upsert=True
                )
                
                if result.upserted_id or result.modified_count > 0:
                    saved_count += 1
            
            logger.info(f"Successfully saved/updated {saved_count} articles")
            return True
            
        except json.JSONDecodeError as e:
            logger.error(f"Error parsing JSON: {str(e)}")
            return False
        except Exception as e:
            logger.error(f"Error saving to database: {str(e)}")
            return False
    
    def process_articles(self):
        """Main function to run C++ script and save articles"""
        logger.info("Starting article processing job...")
        
        # Run C++ script
        json_output = self.run_cpp_script()
        if not json_output:
            logger.error("Failed to get output from C++ script")
            return False
        
        # Save to database
        success = self.save_articles_to_db(json_output)
        if success:
            logger.info("Article processing completed successfully")
        else:
            logger.error("Failed to save articles to database")
        
        return success

# Initialize processor
processor = ArticleProcessor()

# Scheduler setup
scheduler = BackgroundScheduler()
scheduler.add_job(
    func=processor.process_articles,
    trigger="interval",
    hours=2,
    id='article_scraper_job'
)

@app.route('/')
def home():
    """Health check endpoint"""
    return jsonify({
        "status": "running",
        "message": "Article scraper API is active",
        "next_run": scheduler.get_job('article_scraper_job').next_run_time.isoformat() if scheduler.get_job('article_scraper_job') else None
    })

@app.route('/articles')
def get_articles():
    """Get all articles from database"""
    try:
        articles = list(processor.collection.find({}, {"_id": 1, "date": 1, "saved_at": 1}).limit(100))
        return jsonify({
            "count": len(articles),
            "articles": articles
        })
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/articles/<article_id>')
def get_article(article_id):
    """Get specific article by ID"""
    try:
        article = processor.collection.find_one({"_id": article_id})
        if article:
            return jsonify(article)
        else:
            return jsonify({"error": "Article not found"}), 404
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/build')
def build_cpp():
    """Manually trigger C++ project build"""
    try:
        success = processor.build_cpp_project()
        return jsonify({
            "success": success,
            "message": "Build completed successfully" if success else "Build failed"
        })
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/run-now')
def run_now():
    """Manually trigger article processing"""
    try:
        success = processor.process_articles()
        return jsonify({
            "success": success,
            "message": "Job completed successfully" if success else "Job failed"
        })
    except Exception as e:
        return jsonify({"error": str(e)}), 500

@app.route('/stats')
def get_stats():
    """Get database statistics"""
    try:
        total_articles = processor.collection.count_documents({})
        latest_article = processor.collection.find_one(
            {}, 
            sort=[("saved_at", -1)]
        )
        
        return jsonify({
            "total_articles": total_articles,
            "latest_article_saved": latest_article.get("saved_at").isoformat() if latest_article else None,
            "database": DATABASE_NAME,
            "collection": COLLECTION_NAME
        })
    except Exception as e:
        return jsonify({"error": str(e)}), 500

if __name__ == '__main__':
    # Start the scheduler
    scheduler.start()
    logger.info("Scheduler started - articles will be processed every 2 hours")
    
    # Run once at startup
    processor.process_articles()
    
    try:
        app.run(debug=True, host='0.0.0.0', port=5000, use_reloader=False)
    except KeyboardInterrupt:
        logger.info("Shutting down...")
        scheduler.shutdown()