#pragma once
#include "parser.hpp"
#include <raylib.h>

class GameManager{
	public:
		static GameManager* getInstance();
		GameManager();
		void run();
		void unloadGame();
		void loadGame(std::string filename);
		void destroyHitObject(int index);
		void destroyDeadHitObject(int index);
		void render();
		float windowScale = 2.0f;
		int skip = 1;
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
		Texture2D reverseArrow;
		Texture2D sliderin;
		Texture2D sliderout;
		Texture2D numbers[10];
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
		timingSettings timingSettingsForHitObject;
		int index;
		float circlesize = 54.48*2.0f;
	private:
		static GameManager* inst_;
		void init();
		void update();
		void spawnHitObject(HitObjectData data);
		void render_points();
		void render_combo();
		
};
