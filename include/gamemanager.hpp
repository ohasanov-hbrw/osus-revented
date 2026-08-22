#pragma once
#include "parser.hpp"
#include <raylib.h>
#include "linkedListImpl.hpp"
#include "time_util.hpp"
#include <deque>

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

//enum OSU_OBJECT_TYPE {
//	
//};

enum OSU_POINTS_ENUM {
	OSU_300 = 3,
	OSU_100 = 2,
	OSU_50 = 1,
	OSU_0 = 0
};

class GameManager{
	public:
		static GameManager* getInstance();
		GameManager();
		void run();
		void unloadGame();
		void loadGame(std::string filename);
		void loadDefaultSkin(std::string filename);
		void loadGameSkin(std::string filename);
		void loadBeatmapSkin(std::string filename);
		void loadBeatmapSound(std::string filename);
		void loadGameTextures();
		void loadGameSounds();
		void unloadGameTextures();
		void destroyHitObject(Node *node);
		void destroyDeadHitObject(Node *node);
		void unloadSliderTextures();
		//std::vector<int> sliderPreInit(HitObjectData data);
		int * sliderPreInit(HitObjectData data);
		void render();

		float clip(float value, float min, float max);

		GameFile gameFile;
		Parser parser;
		timingSettings currentTimingSettings;

		Linkedlist objectsLinkedList;
		Linkedlist deadObjectsLinkedList;

		std::vector<HitObject*> objects;
		std::vector<FollowPoint> followLines;
		std::vector<HitObject*> dead_objects;
		std::deque<timingSettings> timingSettingsForHitObject;
		std::vector<int8_t> objectPoints;

		HitSound SoundFilesAll;
		
		Background backgroundTextures;
		
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
		Texture2D sliderblank;
		Texture2D sliderout;
		Texture2D numbers[10];
		Texture2D spinnerBottom;
		Texture2D spinnerTop;
		Texture2D spinnerCircle;
		Texture2D spinnerApproachCircle;
		Texture2D spinnerMetre;
		Texture2D spinnerBack;
		Texture2D followPoint;

		RenderTexture2D sliderInnerBall;
		RenderTexture2D sliderOuterBall;

		Music backgroundMusic;

		Color comboColour;

		double currentTime = 0;
		double currentTimeTemp = -1;
		double sliderSpeed = 1.0f;
		double sliderSpeedOverride = 1.0f;
		double verytempbeat2 = 0;
		double verytempbeat = 0;
		double TimerLast = 0;
		double TimeLast = 0;
		double lastHitTime = 0;

		long long int score = 0;
		long long int animatedScore = 0;

		Vector2 MousePosition = {0,0};
		Vector2 lastCords = {0,0};
		
		char *musicData = NULL;
		long musicSize = 0;

		#ifdef THREEDS_BUILD
			int skip = 10;
			int smallskip = 2;
		#endif
		#ifndef THREEDS_BUILD
			int skip = 3;
			int smallskip = 1;
		#endif
		int currentComboIndex = 0;
		int time = 0;
		int meter = 0;
		int combo = 1;
		int clickCombo = 0;
		int maxCombo = 0;
		int sampleSet = 0;
		int sampleIndex = 0;
		int volume = 0;
		int effects = 0;
		int defaultSampleSet = 0;
		int index = 0;
		int lastCurrentTiming = 0;
		int spawnedHitObjects = 0;
		int hit300s = 0;
		int hit100s = 0;
		int hit50s = 0;
		int hit0s = 0;
		int lastTimingLoc = 0;

		float difficultyMultiplier = 0;
		float windowScale = 2.0f;
		float beatLength = 0;
		float slidertickrate = 1.0f;
		float angle = 0;
		float circlesize = 54.48*2.0f;
		float spinsPerSecond = 5.0f;
		
		bool uninherited = false;
		bool pressed = false;
		bool down = false;
		bool stop = false;
		bool renderSpinnerCircle = false;
		bool renderSpinnerMetre = false;
		bool renderSpinnerBack = false;
		bool temprenderSpinnerCircle = false;
		bool temprenderSpinnerMetre = false;
		bool temprenderSpinnerBack = false;
		bool startMusic = false;
		
		std::string currentBackgroundTexture = "";
		std::string lastPath = "";
		std::string BeatmapFolderPathWithSlash = "";
	private:
		static GameManager* inst_;
		void init();
		void update();
		void spawnHitObject(HitObjectData data);
		void render_points();
		void render_combo();
		
};
