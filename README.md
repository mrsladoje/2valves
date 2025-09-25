# 🛠️ 2valves - Serbian Media Analysis

*Exposing the parallel realities of regime vs. independent media coverage*

## Why "2valves"? 🤔

Ah, dear reader, let me pump some knowledge into you! You know how Serbian protests are symbolized by the trusty pump (PUMPAJ), right? Well, this project does something beautifully parallel:

1. We PUMP data using **parallel** scraping (TBB threading magic)
2. We expose **parallel** realities (regime vs independent media narratives)

So naturally, our metaphorical pump needs **2valves** - one for each reality stream! Get it? It's like we're pumping truth from two different wells simultaneously. Sometimes the puns just write themselves! 🚰

## What Does This Beast Do? 🤯

2valves is a full-stack application that scrapes Serbian media in parallel and lets you see how different outlets cover the same events. It's like having X-ray vision for media bias, except instead of seeing through walls, you're seeing through propaganda.

**The Cast:**
- **Independent Media**: 021.rs, N1info.rs (the good guys)
- **Regime Media**: Kurir.rs, Informer.rs (the... other guys, basically Goebbels' wet dream)

## Technical Architecture (Don't Worry, We'll Keep It Light) 🤖

### The Stack That (Hopefully 😎) Won't Collapse:

**Backend**: Flask with a CRON job that's more reliable than Serbian public transport
- Runs every few hours to scrape fresh content
- Stores everything in MongoDB
- Uses Gemini AI for objective analysis 

**Scraping Engine**: C++ with Intel TBB (Threading Building Blocks)
- Parallel scraping because life's too short for sequential requests
- Hierarchical parallelization (fancy words for "really fast")
- Each media outlet gets its own thread party

**Frontend**: Svelte with neobrutalist styling
- Because sometimes brutal honesty needs brutal design
- Clean, functional, and won't hurt your eyes

## Setup Instructions 🎰

Ah, the setup. Here's where things get spicy!

Run this and pray to whatever deities handle dependency management:

```bash
python setup.py
```

### Important Reality Check:

**IF THIS WORKS ON YOUR FIRST TRY, DROP EVERYTHING AND GO BUY A LOTTERY TICKET RIGHT NOW.** 🤣🤣🤣🤣🤣

Seriously. Claude (I like to call him "kum") and I are probably the only two entities in the known universe who have successfully run this project. And even then:
- ✅ Works on my main PC
- ❌ Doesn't work on my laptop (because apparently my laptop has trust issues)
- ❌ Probably won't work on your machine (but hey, prove us wrong!)

The setup.py is *theoretically* platform-agnostic and *should* handle all the heavy lifting. We've poured our silicon hearts into making this work across different systems. But let's be real - modern software dependency management is like trying to solve a Rubik's cube while riding a unicycle on fire.

## Probably your only chance to see this in action (▶️YouTube🔴 Preview!!!)

[![Wow, how cool!!!!](https://img.youtube.com/vi/4mTsmEurYD4/maxresdefault.jpg)](https://www.youtube.com/watch?v=4mTsmEurYD4)


### What You'll Need (In Theory):

- Python 3.8+
- Node.js (for the Svelte frontend)
- C++ compiler with C++17 support
- vcpkg (for C++ dependencies)
- CMake
- A MongoDB instance
- Gemini API key
- Patience (lots of it)
- Strong coffee
- Maybe some tissues for frustrated tears

## Project Structure (For The Brave🦁 Souls)

```
2valves/
├── cpp-script/           # C++ parallel scraping engine
├── flask-api/           # Python backend with CRON jobs
├── frontend/            # Svelte neobrutalist UI
├── materials/           # Documentation (you're reading part of it!)
└── setup.py            # The script that will either save or doom you
```

## Features That Actually😇 Work: 

- ⚡ Parallel web scraping (up to 4x faster than sequential, assuming your CPU doesn't give up)
- 🤖 AI-powered media analysis (because humans are biased, but AI is... differently biased?)
- 📊 Side-by-side comparison of media narratives
- 🎨 Neobrutalist UI that's honest about being brutally functional
- 🔄 Automatic updates via CRON jobs
- 💾 MongoDB persistence (your data survives even if your sanity doesn't)

## Contributing 🤥

Want to contribute? That's pump-tastic! But first, you'll need to actually get this thing running. If you manage that feat, you're automatically qualified to be a maintainer.

## License 🥵

This project is licensed under the "Good Luck Getting It To Work" license. Use it, abuse it, learn from it, cry over it - just don't blame us when setup.py gives you trust issues.

## Final Words 🤧

Remember, this project isn't just about code - it's about pumping transparency into media consumption. Whether you're team independent media or you're just here for the parallel processing puns, 2valves is here to help you see both sides of the story.

Now go forth, brave developer, and may your compilations be swift and your dependencies forever resolved! 🚀

*P.S.: If you actually get this running on the first try, hmu, I'll buy you a beer.*
