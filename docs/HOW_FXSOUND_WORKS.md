# How FxSound Works: Under the Hood

FxSound is designed to do one simple thing: make whatever audio comes out of your computer sound louder, clearer, deeper, and more immersive—all in real time, without noticeable lag. 

Whether you are listening to Spotify, watching YouTube, gaming, or jumping on a voice call, FxSound acts as a personal sound engineer sitting between your favorite apps and your physical speakers or headphones.

Here is a step-by-step look at how the entire system works from the moment a sound is made to the moment it hits your ears.

---

## 1. The Big Picture: The "Middleman" Trick

Normally, when you press play in an app like Spotify or Firefox, your computer sends raw sound waves straight to your physical sound card or headphones. 

FxSound pulls off a clever trick by acting as a friendly middleman:

1. **The Virtual Speaker:** When FxSound opens, it tells your computer to create a pretend speaker named **"FxSound Audio Enhancer"** (`support.null-audio-sink` created using PipeWire in `linux/sndDevices_pw.cpp`). It politely asks your desktop to make this pretend speaker the default output.
2. **Catching the Sound:** All your apps send their audio to this pretend speaker without realizing anything has changed. Instead of playing it out loud, FxSound silently intercepts the audio data.
3. **The Assembly Line:** FxSound scoops up the audio in tiny, bite-sized slices lasting only 20 milliseconds each. It sends these slices down a lightning-fast memory conveyor belt (`lock-free SPSC ring buffer` in `linux/sndDevices_pw.cpp`) into its dedicated sound-shaping engine (`AudioPassthru_linux.cpp`).
4. **Passing It On:** After the sound engine finishes polishing the audio, it sends the enhanced sound right out to your real physical headphones, desktop speakers, or USB DAC (`pw-playback` stream targeting your real hardware sink).

Because this whole journey takes only a few milliseconds, your brain hears the enhanced sound instantly with zero noticeable delay or lip-sync lag when watching movies or playing games.

---

## 2. Inside the Audio Engine: What the 5 Effects Actually Do

Once your audio enters the sound workshop (`libDfxDsp` in the `dsp/` folder), the sound wave is analyzed and shaped through a series of specialized digital audio filters. 

FxSound gives you 5 core effect sliders. Here is what each one actually does in plain terms:

### 1. Clarity (Crispness & Detail)
- **What you hear:** Vocals become intelligible, acoustic guitar strings sparkle, and snare drums snap cleanly into focus.
- **How it works:** Many audio files or streams lose high-frequency brilliance when compressed (like MP3s or online video streams). The Clarity effect (`Aural` harmonic exciter in `dsp/ptechDsp/Aural/`) analyzes high frequencies and synthesizes subtle harmonic overtones. Rather than just making the treble uncomfortably loud or shrill, it gently restores the natural "air" and presence of the original recording (like wiping dust off a clean glass window).

### 2. Ambience (Depth & Room Feel)
- **What you hear:** Sounds stop feeling dry and trapped inside your head; they feel like they are playing in a warm, natural listening room.
- **How it works:** Pure digital audio can sound sterile because real-world sounds always bounce off walls, ceilings, and furniture before reaching your ears. The Ambience slider (`Lex` spatial reverb in `dsp/ptechDsp/Lex/`) injects micro-reflections that simulate the warmth of an acoustic listening room without sounding like you are shouting inside a cathedral.

### 3. Surround Sound (Wide Stereo Image)
- **What you hear:** The soundstage expands outward, feeling significantly wider than the physical distance between your left and right speakers or headphone earcups.
- **How it works:** Human ears detect where sounds come from by noticing the microscopic arrival-time differences between the left and right ear. The Surround Sound effect (`wide` stereo shaper in `dsp/ptechDsp/wide/`) gently shifts the phase relationships between the two audio channels, convincing your brain that sound is wrapping around your room.

### 4. Dynamic Boost (Fullness, Punch & Volume)
- **What you hear:** Quiet dialogue becomes easy to hear, loud explosions don't blow out your eardrums, and your total volume gets a clean boost without harsh buzzing or clipping.
- **How it works:** The Dynamic Boost tool (`Maximizer` peak limiter and auto-leveler in `dsp/ptechDsp/Maximizer/`) constantly rides the volume levels in fractions of a second. It lifts the quiet, buried details while keeping a firm ceiling on loud peaks, preventing painful digital distortion and giving you maximum perceived volume.

### 5. Bass Boost (Deep, Warm Thump)
- **What you hear:** A tight, satisfying bass rumble in kick drums, bass guitars, and action movie explosions that doesn't drown out people's voices.
- **How it works:** Instead of blindly cranking the low frequencies (which makes desktop speakers buzz and voices sound muddy), the Bass Boost processor (`Dly` resonant sub-harmonic generator in `dsp/ptechDsp/Dly8/`) calculates the fundamental pitch of low notes and reinforces them with tuned harmonics. This lets smaller speakers and headphones produce a punchy low-end feel they couldn't normally deliver.

---

## 3. The Equalizer (EQ) & Presets

### The 9-Band Equalizer
While the 5 effect sliders shape the character and dynamics of the sound, the Equalizer gives you direct control over specific frequency tones (`parametric biquad filter banks` in `dsp/ptechDsp/Peq/`):

- **Low Frequencies (32 Hz to 125 Hz):** Deep rumble, sub-bass, and kick drum thump.
- **Mid Frequencies (250 Hz to 2 kHz):** Warmth, guitar bodies, piano notes, and spoken vocals.
- **High Frequencies (4 kHz to 16 kHz):** Vocal clarity, cymbals, breathiness, and high-end sparkle.

Moving a slider up boosts that specific tone; moving it down cuts it back to compensate for bright headphones or boomy rooms.

### Presets: Ready-to-Go Sound Recipes
You don't have to tweak every slider by hand. FxSound bundles presets—curated recipes combining specific EQ curves and effect levels (`.fac` preset files in `~/.config/FxSound/Presets/`):
- **Music:** Balanced clarity, deep bass, and wide stereo for general music listening.
- **Voice:** Boosted mid-frequencies and dynamic leveling so podcasts and meetings are effortlessly understandable.
- **Gaming:** Enhanced spatial cues and footsteps with dynamic control to pinpoint in-game sounds.
- **Bass Boost:** Tuned low-end resonance for electronic, hip-hop, and action cinema.

---

## 4. The Interface & Controls: How You Drive It

FxSound provides three ways to interact with the sound engine:

1. **The Visual Desktop App (`fxsound/Source/GUI/` using JUCE):**
   The main window lets you drag sliders, toggle the giant power button, switch presets, and watch a live spectrum visualizer dancing to your audio (`FxVisualizer.cpp`). Whenever you touch a slider, the interface sends an instant message to the background DSP thread, updating your sound smoothly without pops, clicks, or dropouts.
2. **The System Tray Icon (`linux/tray_sni.cpp`):**
   When you close the FxSound window, it doesn't shut down your audio. It quietly slips into your Linux system tray (`StatusNotifierItem` over D-Bus). Left-clicking brings the window back, middle-clicking toggles power, wheel-scrolling changes presets, and right-clicking gives you a quick menu.
3. **The Command Line & Keyboard Shortcuts (`$XDG_RUNTIME_DIR/fxsound-cli.sock` and D-Bus):**
   You can automate FxSound or trigger it from custom keyboard shortcuts without opening the visual window:
   ```bash
   # Toggle power on/off instantly
   FxSound --power=toggle

   # Switch to your gaming preset
   FxSound --preset="Gaming"
   ```
   A lightweight socket receiver (`linux/single_instance_posix.cpp`) catches these commands and applies them to the running engine in a fraction of a millisecond.
