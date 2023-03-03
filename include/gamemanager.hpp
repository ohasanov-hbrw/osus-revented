#pragma once
#include "parser.hpp"
#include <raylib.h>

struct dbool
{
    bool value = false;
};

struct HitSound {
    std::map<std::string, Sound> data;
	std::map<std::string, dbool> loaded;
};

struct Background {
    std::map<std::string, Texture2D> data;
	std::map<std::string, Vector2> pos;
	std::map<std::string, dbool> loaded;
};

class GameManager{
	public:
		static GameManager* getInstance();
		GameManager();
		void run();
		void unloadGame();
		void loadGame(std::string filename);
		void loadDefaultSkin(std::string filename);
		void loadDefaultSound(std::string filename);
		void loadGameSkin(std::string filename);
		void loadGameSound(std::string filename);
		void loadBeatmapSkin(std::string filename);
		void loadBeatmapSound(std::string filename);
		void loadGameTextures();
		void unloadGameTextures();
		void destroyHitObject(int index);
		void destroyDeadHitObject(int index);
		void render();
		float windowScale = 2.0f;
        HitSound hitCircleHS;
		int skip = 2;
		bool renderSpinnerCircle = false;
		bool renderSpinnerMetre = false;
		bool renderSpinnerBack = false;
		Texture2D hitCircle;
		Texture2D hitCircleOverlay;
		Texture2D approachCircle;
		Texture2D cursor;
		Texture2D selectCircle;
		Texture2D hit0;
		Texture2D hit50;
		Texture2D hit100;
		Texture2D hit300;
		Texture2D sliderb;
		Texture2D sliderscorepoint;
		Texture2D sliderfollow;
		Texture2D reverseArrow;
		Texture2D sliderin;
		Texture2D sliderout;
		Texture2D numbers[10];
		Texture2D spinnerBottom;
		Texture2D spinnerTop;
		Texture2D spinnerCircle;
		Texture2D spinnerApproachCircle;
		Texture2D spinnerMetre;
		Texture2D spinnerBack;
		Music backgroundMusic;
		double currentTime;
		double currentTimeTemp = -1;
		int combo = 1;
		int clickCombo = 0;
		Color comboColour;
		int score = 0;
		float difficultyMultiplier = 0;
		int currentComboIndex = 0;
		int time;
		int meter;
		float beatLength;
		float slidertickrate = 1.0f;
		int sampleSet;
		int sampleIndex;
		int volume;
		bool uninherited;
		int effects;
		double sliderSpeed = 1.0f;
		double sliderSpeedOverride = 1.0f;
		bool pressed = false;
		bool down = false;
		float angle = 0;
		bool stop = false;
		Vector2 MousePosition;
		GameFile gameFile;
		Parser parser;
		std::vector<HitObject*> objects;
		std::vector<HitObject*> dead_objects;
		float clip(float value, float min, float max);
		std::vector<timingSettings> timingSettingsForHitObject;
		int index;
		float circlesize = 54.48*2.0f;
		double verytempbeat;
		float spinsPerSecond = 5.0f;
		int spawnedHitObjects = 0;
		bool startMusic = false;
		double TimerLast = 0;
		double TimeLast = 0;
		HitSound SoundFiles;


		Background backgroundTextures;
		std::string currentBackgroundTexture = "";

		bool temprenderSpinnerCircle = false;
		bool temprenderSpinnerMetre = false;
		bool temprenderSpinnerBack = false;

		std::string lastPath;
		

		char *musicData;
		long musicSize;
	private:
		static GameManager* inst_;
		void init();
		void update();
		void spawnHitObject(HitObjectData data);
		void render_points();
		void render_combo();
		
};
