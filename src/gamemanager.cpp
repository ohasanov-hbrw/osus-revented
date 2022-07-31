#include "gamemanager.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <vector>
#include <math.h>
#include <hitobject.hpp>
#include "globals.hpp"
#include "utils.hpp"
#include "fs.hpp"

//for some reason the clamp function didnt work so here is a manual one
float GameManager::clip(float value, float min, float max){
  	return std::min(std::max(value,min), max);
}


GameManager* GameManager::inst_ = NULL;

//get a new gamemanager
GameManager* GameManager::getInstance() {
   	if (inst_ == NULL)
   		inst_ = new GameManager();
   	return(inst_);
}

//call the initilization
GameManager::GameManager(){
	init();
}

//initilize the game manager
void GameManager::init(){
	//initilize the audio part of raylib
	
	//initilize the window
	
	//set the fps to the common number of 60
	
	//hide the cursor because we have a custom one
	
	//load all the textures (can also do this in load_game)
    

}

//main game loop
void GameManager::update(){
	//update the music an  d get the time from it
	
	//get the mouse position and state
	
	//currently not used that much but it will be
	//timingSettingsForHitObject.clear();
	int timingSize = gameFile.timingPoints.size();
	for(int i = timingSize-1; i >= 0; i--){
		if(gameFile.timingPoints[i].time - gameFile.preempt <= currentTime*1000){
			timingSettings tempTiming;
			tempTiming.renderTicks = gameFile.timingPoints[i].renderTicks;
			tempTiming.sliderSpeedOverride = 1;
			tempTiming.time = gameFile.timingPoints[i].time;
			double tempBeatLength;
			tempBeatLength = gameFile.timingPoints[i].beatLength;
			//std::cout << "beatLength: " << tempBeatLength << std::endl;
			if(tempBeatLength >= 0){
				tempTiming.beatLength = tempBeatLength;
				verytempbeat = tempBeatLength;
				tempTiming.sliderSpeedOverride = 1;
			}
			if(tempBeatLength < 0){
				tempTiming.sliderSpeedOverride = (100 / tempBeatLength * (-1));
				tempTiming.beatLength = verytempbeat;
			}
			tempTiming.meter = gameFile.timingPoints[i].meter;
			tempTiming.sampleSet = gameFile.timingPoints[i].sampleSet;
			tempTiming.sampleIndex = gameFile.timingPoints[i].sampleIndex;
			tempTiming.volume = gameFile.timingPoints[i].volume;
			tempTiming.uninherited = gameFile.timingPoints[i].uninherited;
			tempTiming.effects = gameFile.timingPoints[i].effects;
			timingSettingsForHitObject.push_back(tempTiming);
			gameFile.timingPoints.pop_back();
		}
		else
			break;
	}
	
	//spawn the hitobjects when their time comes
	int size = gameFile.hitObjects.size();	
	for(int i = size-1; i >= 0; i--){
		if(gameFile.hitObjects[i].time - gameFile.preempt <= currentTime*1000){
			spawnHitObject(gameFile.hitObjects[i]);
			if(objects[objects.size()-1]->data.startingACombo){
				currentComboIndex++;
				if(gameFile.comboColours.size()) currentComboIndex = (currentComboIndex + objects[objects.size()-1]->data.skipComboColours) % gameFile.comboColours.size();
				combo = 1;
			}
			if(gameFile.comboColours.size()) objects[objects.size()-1]->data.colour = gameFile.comboColours[currentComboIndex];
			objects[objects.size()-1]->data.comboNumber = combo;
			combo++;
			int index = 0;
			for(int amog = 0; amog < timingSettingsForHitObject.size(); amog++){
				if(timingSettingsForHitObject[amog].time > gameFile.hitObjects[i].time)
					break;
				index = amog;
			}
			
			objects[objects.size()-1]->data.timing.beatLength = timingSettingsForHitObject[index].beatLength;
			objects[objects.size()-1]->data.timing.meter = timingSettingsForHitObject[index].meter;
			objects[objects.size()-1]->data.timing.sampleSet = timingSettingsForHitObject[index].sampleSet;
			objects[objects.size()-1]->data.timing.sampleIndex = timingSettingsForHitObject[index].sampleIndex;
			objects[objects.size()-1]->data.timing.volume = timingSettingsForHitObject[index].volume;
			objects[objects.size()-1]->data.timing.uninherited = timingSettingsForHitObject[index].uninherited;
			objects[objects.size()-1]->data.timing.effects = timingSettingsForHitObject[index].effects;
			objects[objects.size()-1]->data.timing.sliderSpeedOverride = timingSettingsForHitObject[index].sliderSpeedOverride;
			objects[objects.size()-1]->data.index = objects.size()-1;
			objects[objects.size()-1]->data.timing.renderTicks = timingSettingsForHitObject[index].renderTicks;
			/*std::cout << "Time:" << timingSettingsForHitObject[index].time << " Beat:" << objects[objects.size()-1]->data.timing.beatLength <<
			" Meter:" << objects[objects.size()-1]->data.timing.meter << " SV:" << objects[objects.size()-1]->data.timing.sliderSpeedOverride <<
			" SS:" << sliderSpeed << " RT:" << objects[objects.size()-1]->data.timing.renderTicks;*/
			objects[objects.size()-1]->init();
			gameFile.hitObjects.pop_back();
			for(int amog = 0; amog < index - 1; amog++){
				timingSettingsForHitObject.erase(timingSettingsForHitObject.begin());
			}
		}
		else
			break;
	}
	//update and check collision for every hit circle
	int newSize = objects.size();
	int oldSize = objects.size();
	int susSize = objects.size();
	bool stop = true;
	for(int i = susSize-1; i >= 0; i--){
		//if((std::abs(currentTime*1000 - objects[i]->data.time) <= gameFile.p50Final)){
		if(i == 0){
			objects[i]->data.touch = true;
		}
		if (stop && i == 0 && (Global.Key1P or Global.Key2P)){
			if (objects[i]->data.type != 2){
				if (CheckCollisionPointCircle(Global.MousePosition,Vector2{objects[i]->data.x,(float)objects[i]->data.y}, circlesize/2.0f)){
                    switch(objects[i]->data.hitSound) {
                        case 1: {
                            PlaySound(hitCircleHS.normal);
                        } break;
                        case 2: {
                            PlaySound(hitCircleHS.whistle);
                        } break;
                        case 4: {
                            PlaySound(hitCircleHS.finish);
                        } break;
                        case 8: {
                            PlaySound(hitCircleHS.clap);
                        } break;
                        default: {
                            PlaySound(hitCircleHS.normal);
                        } break;
                    }
					if(std::abs(currentTime*1000 - objects[i]->data.time) > gameFile.p50Final){
						objects[i]->data.point = 0;
						clickCombo = 0;
					}
					else if(std::abs(currentTime*1000 - objects[i]->data.time) > gameFile.p100Final){
						objects[i]->data.point = 1;
						score+= 50 + (50 * (std::max(clickCombo-1,0) * difficultyMultiplier * 1)/25);
						clickCombo++;
					}
					else if(std::abs(currentTime*1000 - objects[i]->data.time) > gameFile.p300Final){
						objects[i]->data.point = 2;
						score+= 100 + (100 * (std::max(clickCombo-1,0) * difficultyMultiplier * 1)/25);
						clickCombo++;
					}
					else{
						objects[i]->data.point = 3;
						score+= 300 + (300 * (std::max(clickCombo-1,0) * difficultyMultiplier * 1)/25);
						clickCombo++;
					}
					objects[i]->data.time = currentTime*1000;
					destroyHitObject(i);
					newSize = objects.size();
					i--;
					stop = false;
				}
				else{
					objects[i]->data.touch = true;
					objects[i]->update();
					newSize = objects.size();
					if(newSize != oldSize){
						i--;
						oldSize = newSize;
					}
				}
			}
			else if (objects[i]->data.type == 2){
				if(Slider* tempslider = dynamic_cast<Slider*>(objects[i]))
					if(CheckCollisionPointCircle(Global.MousePosition,Vector2{objects[i]->data.x,(float)objects[i]->data.y}, circlesize/2.0f) && currentTime*1000 < tempslider->data.time + gameFile.p50Final){
						if(std::abs(currentTime*1000 - tempslider->data.time) > gameFile.p50Final){
							tempslider->is_hit_at_first = true;
							stop = false;
							tempslider->earlyhit = true;
							clickCombo = 0;
						}
						else{
							tempslider->is_hit_at_first = true;
							stop = false;
							clickCombo++;
						}
					}
				//this cursed else train is nothing to worry about...
				objects[i]->data.index = i;
				objects[i]->update();
				newSize = objects.size();
				if(newSize != oldSize){
					i--;
					oldSize = newSize;
				}
			}
			else{
				objects[i]->data.index = i;
				objects[i]->update();
				newSize = objects.size();
				if(newSize != oldSize){
					i--;
					oldSize = newSize;
				}
			}
		}
		else{
			objects[i]->data.index = i;
			objects[i]->update();
			newSize = objects.size();
			if(newSize != oldSize){
				i--;
				oldSize = newSize;
			}
		}
		//}
		/*else{
			objects[i]->data.index = i;
			objects[i]->update();
			newSize = objects.size();
			if(newSize != oldSize){
				i--;
				oldSize = newSize;
			}
		}*/
	}
	//also update the dead objects
	for(size_t i = 0; i < dead_objects.size(); i++){
		dead_objects[i]->data.index = i;
		dead_objects[i]->dead_update();
	}
}

//main rendering loop
void GameManager::render(){
	//set the screen for drawing
	
	//currently the background is a set color but we can change that
	
	//draw the fps
	
	//this is the mouse scale... i think
	
	//render all the objects
	
	for(int i = objects.size() - 1; i >= 0; i--){
		objects[i]->render();
	}
	for(int i = dead_objects.size() - 1; i >= 0; i--){
		dead_objects[i]->dead_render();
	}
	DrawCNumbersCenter(score, 320, 10, 0.4f, GREEN);
	DrawCNumbersCenter(clickCombo, 320, 25, 0.4f, GREEN);
	//render the points and the combo
	
}

void GameManager::run(){
	//start playing the music and set the volume, it gets quite loud
	if(GetMusicTimeLength(backgroundMusic) - GetMusicTimePlayed(backgroundMusic) < 1.0f)
		stop = true;
	if(stop && currentTime < 1.0f)
		StopMusicStream(backgroundMusic);
	UpdateMusicStream(backgroundMusic);
	if(currentTimeTemp != GetMusicTimePlayed(backgroundMusic) && IsMusicStreamPlaying(backgroundMusic)){
		currentTimeTemp = GetMusicTimePlayed(backgroundMusic);
		currentTime = currentTimeTemp;
	}
	else{
		if(IsMusicStreamPlaying(backgroundMusic)){
			currentTime += GetFrameTime();
		}
	}
	GameManager::update();

}

//load the beatmap
void GameManager::loadGame(std::string filename){
	//create a parser and parse the file
	Parser parser = Parser();
	gameFile = parser.parse(filename);
	//reverse the hitobject array because we need it reversed for it to make sense (and make it faster because pop_back)
	std::reverse(gameFile.hitObjects.begin(),gameFile.hitObjects.end());
	std::reverse(gameFile.timingPoints.begin(),gameFile.timingPoints.end());
	//calculate all the variables for the game (these may be a bit wrong but they feel right)
	if(std::stoi(gameFile.configDifficulty["ApproachRate"]) < 5){
		gameFile.preempt = 1200 + 600 * (5 - std::stoi(gameFile.configDifficulty["ApproachRate"])) / 5;
		gameFile.fade_in = 800 + 400 * (5 - std::stoi(gameFile.configDifficulty["ApproachRate"])) / 5;
	}
	else if(std::stoi(gameFile.configDifficulty["ApproachRate"]) > 5){
		gameFile.preempt = 1200 - 750 * (std::stoi(gameFile.configDifficulty["ApproachRate"]) - 5) / 5;
		gameFile.fade_in = 800 - 500 * (std::stoi(gameFile.configDifficulty["ApproachRate"]) - 5) / 5;
	}
	else{
		gameFile.preempt = 1200;
		gameFile.fade_in = 800;
	}
	float od = std::stoi(gameFile.configDifficulty["OverallDifficulty"]);
	if(od < 5){
		spinsPerSecond = 5.0f - 2.0f * (5.0f - od) / 5.0f;
	}
	else if(od > 5){
		spinsPerSecond = 5.0f - 2.0f * (od - 5.0f) / 5.0f;
	}
	else{
		spinsPerSecond = 5.0f;
	}
	gameFile.p300Final = gameFile.p300 - std::stoi(gameFile.configDifficulty["OverallDifficulty"]) * gameFile.p300Change;
	gameFile.p100Final = gameFile.p100 - std::stoi(gameFile.configDifficulty["OverallDifficulty"]) * gameFile.p100Change;
	gameFile.p50Final = gameFile.p50 - std::stoi(gameFile.configDifficulty["OverallDifficulty"]) * gameFile.p50Change;
	//debug, just say what the name of the music file is and load it
	std::cout << (Global.Path + '/' + gameFile.configGeneral["AudioFilename"]) << std::endl;
	backgroundMusic = LoadMusicStream((Global.Path + '/' + gameFile.configGeneral["AudioFilename"]).c_str());
	score = 0;
	clickCombo = 0;
    //these are not used right now, USE THEM
	float hpdrainrate = std::stof(gameFile.configDifficulty["HPDrainRate"]);
	circlesize = 54.4f - (4.48f * std::stof(gameFile.configDifficulty["CircleSize"]));
	slidertickrate = std::stof(gameFile.configDifficulty["SliderTickRate"]);
	circlesize *= 2.0f;
	std::cout << circlesize << std::endl;
	float overalldifficulty = std::stof(gameFile.configDifficulty["OverallDifficulty"]);
	//more difficulty stuff, may also be wrong
	difficultyMultiplier = ((hpdrainrate + circlesize + overalldifficulty + clip((float)gameFile.hitObjects.size() / GetMusicTimeLength(backgroundMusic) * 8.f, 0.f, 16.f)) / 38.f * 5.f);
	if (gameFile.configDifficulty.find("SliderMultiplier") != gameFile.configDifficulty.end())
		sliderSpeed = std::stof(gameFile.configDifficulty["SliderMultiplier"]);

	std::string lastPath = Global.Path;

	Global.Path = "resources/default_skin/";
	std::vector<std::string> files = ls(".png");

	std::sort(files.begin(), files.end(), []
    (const std::string& first, const std::string& second){
        return first.size() < second.size();
    });
	std::reverse(files.begin(), files.end());

	for(int i = 0; i < files.size(); i++){
		if(IsFileExtension(files[i].c_str(),".png")){
			if(files[i].rfind("hitcircleoverlay.png", 0) == 0)
				hitCircleOverlay = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hitcircleselect.png", 0) == 0)
				selectCircle = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hitcircle", 0) == 0)
				hitCircle = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("approachcircle", 0) == 0)
				approachCircle = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hit300k", 0) == 0)
				;
			else if(files[i].rfind("hit300", 0) == 0)
				hit300 = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hit100k", 0) == 0)
				;
			else if(files[i].rfind("hit100", 0) == 0)
				hit100 = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hit50k", 0) == 0)
				;
			else if(files[i].rfind("hit50", 0) == 0)
				hit50 = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hit0", 0) == 0)
				hit0 = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("sliderscorepoint", 0) == 0)
				sliderscorepoint = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("sliderfollowcircle", 0) == 0)
				sliderfollow = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("sliderb0", 0) == 0)
				sliderb = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("reversearrow", 0) == 0)
				reverseArrow = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("spinner-circle", 0) == 0){
				spinnerCircle = LoadTexture((Global.Path + files[i]).c_str());
				//renderSpinnerCircle = true;
			}
			else if(files[i].rfind("spinner-metre", 0) == 0){
				spinnerMetre = LoadTexture((Global.Path + files[i]).c_str());
				//renderSpinnerMetre = true;
			}
			else if(files[i].rfind("spinner-bottom", 0) == 0)
				spinnerBottom = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("spinner-top", 0) == 0)
				spinnerTop = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("spinner-approachcircle", 0) == 0)
				spinnerApproachCircle = LoadTexture((Global.Path + files[i]).c_str());
			else{
				for(int j = 0; j < 10; j++){
					if(files[i].rfind(("default-" + (std::to_string(j))).c_str(), 0) == 0){
    					numbers[j] = LoadTexture((Global.Path + files[i]).c_str());
					}
				}
			}
		}
	}

	renderSpinnerCircle = false;
	renderSpinnerMetre = false;

	files.clear();
	//Global.Path = "resources/skin/";
	files = ls(".png");

	std::sort(files.begin(), files.end(), []
    (const std::string& first, const std::string& second){
        return first.size() < second.size();
    });
	std::reverse(files.begin(), files.end());

	for(int i = 0; i < files.size(); i++){
		if(IsFileExtension(files[i].c_str(),".png")){
			if(files[i].rfind("hitcircleoverlay.png", 0) == 0)
				hitCircleOverlay = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hitcircleselect.png", 0) == 0)
				selectCircle = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hitcircle", 0) == 0)
				hitCircle = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("approachcircle", 0) == 0)
				approachCircle = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hit300k", 0) == 0)
				;
			else if(files[i].rfind("hit300", 0) == 0)
				hit300 = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hit100k", 0) == 0)
				;
			else if(files[i].rfind("hit100", 0) == 0)
				hit100 = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hit50k", 0) == 0)
				;
			else if(files[i].rfind("hit50", 0) == 0)
				hit50 = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("hit0", 0) == 0)
				hit0 = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("sliderscorepoint", 0) == 0)
				sliderscorepoint = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("sliderfollowcircle", 0) == 0)
				sliderfollow = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("sliderb0", 0) == 0)
				sliderb = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("reversearrow", 0) == 0)
				reverseArrow = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("spinner-circle", 0) == 0){
				spinnerCircle = LoadTexture((Global.Path + files[i]).c_str());
				renderSpinnerCircle = true;
			}
			else if(files[i].rfind("spinner-metre", 0) == 0){
				spinnerMetre = LoadTexture((Global.Path + files[i]).c_str());
				renderSpinnerMetre = true;
			}
			else if(files[i].rfind("spinner-bottom", 0) == 0)
				spinnerBottom = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("spinner-top", 0) == 0)
				spinnerTop = LoadTexture((Global.Path + files[i]).c_str());
			else if(files[i].rfind("spinner-approachcircle", 0) == 0)
				spinnerApproachCircle = LoadTexture((Global.Path + files[i]).c_str());
			else{
				for(int j = 0; j < 10; j++){
					if(files[i].rfind(("default-" + (std::to_string(j))).c_str(), 0) == 0){
    					numbers[j] = LoadTexture((Global.Path + files[i]).c_str());
					}
				}
			}
		}
	}

	files.clear();
	Global.Path = "resources/skin/";
	files = ls(".wav");

	std::sort(files.begin(), files.end(), []
    (const std::string& first, const std::string& second){
        return first.size() < second.size();
    });
	std::reverse(files.begin(), files.end());
    for(const auto& file : files) {
        auto final_path = (Global.Path + file);
        if(file == "drum-hitclap.wav") {
            hitCircleHS.clap = LoadSound(final_path.c_str());
        }
        else if(file == "drum-hitfinish.wav") {
            hitCircleHS.finish = LoadSound(final_path.c_str());
        }
        else if(file == "drum-hitnormal.wav") {
            hitCircleHS.normal = LoadSound(final_path.c_str());
        }
        else if(file == "drum-hitwhistle.wav") {
            hitCircleHS.whistle = LoadSound(final_path.c_str());
        }
    }


	sliderin = LoadTexture("resources/sliderin.png");
	sliderout = LoadTexture("resources/sliderout.png");

	Global.Path = lastPath;
	GenTextureMipmaps(&hit0);
	SetTextureFilter(hit0, TEXTURE_FILTER_TRILINEAR );
	GenTextureMipmaps(&hit50);
	SetTextureFilter(hit50, TEXTURE_FILTER_TRILINEAR );
	GenTextureMipmaps(&hit100);
	SetTextureFilter(hit100, TEXTURE_FILTER_TRILINEAR );
	GenTextureMipmaps(&hit300);
	SetTextureFilter(hit300, TEXTURE_FILTER_TRILINEAR );
	GenTextureMipmaps(&approachCircle);
	SetTextureFilter(approachCircle, TEXTURE_FILTER_TRILINEAR );
	GenTextureMipmaps(&selectCircle);
	SetTextureFilter(selectCircle, TEXTURE_FILTER_TRILINEAR );
	GenTextureMipmaps(&hitCircleOverlay);
	SetTextureFilter(hitCircleOverlay, TEXTURE_FILTER_TRILINEAR );
	GenTextureMipmaps(&hitCircle);
	SetTextureFilter(hitCircle, TEXTURE_FILTER_TRILINEAR );
	GenTextureMipmaps(&sliderb);
	SetTextureFilter(sliderb, TEXTURE_FILTER_TRILINEAR );
	GenTextureMipmaps(&sliderin);
	SetTextureFilter(sliderin, TEXTURE_FILTER_TRILINEAR );
	GenTextureMipmaps(&sliderscorepoint);
	SetTextureFilter(sliderscorepoint, TEXTURE_FILTER_TRILINEAR );
	GenTextureMipmaps(&sliderout);
	SetTextureFilter(sliderout, TEXTURE_FILTER_TRILINEAR );
	GenTextureMipmaps(&reverseArrow);
	SetTextureFilter(reverseArrow, TEXTURE_FILTER_TRILINEAR );
	for(int i = 0; i < 10; i++){
		GenTextureMipmaps(&numbers[i]);
		SetTextureFilter(numbers[i], TEXTURE_FILTER_TRILINEAR );
	}
	PlayMusicStream(backgroundMusic);
    SetMusicVolume(backgroundMusic, 0.2f);
	stop = false;
}

void GameManager::unloadGame(){
	UnloadTexture(hitCircleOverlay);
	UnloadTexture(selectCircle);
	UnloadTexture(hitCircle);
	UnloadTexture(sliderscorepoint);
	UnloadTexture(approachCircle);
	UnloadTexture(hit300);
	UnloadTexture(hit100);
	UnloadTexture(hit50);
	UnloadTexture(hit0);
	UnloadTexture(sliderb);
	UnloadTexture(sliderin);
	UnloadTexture(sliderout);
	UnloadTexture(reverseArrow);
	UnloadTexture(spinnerBottom);
	UnloadTexture(spinnerTop);
	UnloadTexture(spinnerCircle);
	UnloadTexture(spinnerApproachCircle);
	UnloadTexture(spinnerMetre);
	for(int i = 0; i < 10; i++){
		UnloadTexture(numbers[i]);
	}
	objects.clear();
	dead_objects.clear();
}

void GameManager::spawnHitObject(HitObjectData data){
	//spawn a new hitobject, the cool way
	HitObject *temp;
	if(data.type == 1){
		temp = new Circle(data);
		objects.push_back(temp);
	}
	else if(data.type == 2){
		temp = new Slider(data);
		objects.push_back(temp);

	}
	else if(data.type == 3){
		temp = new Spinner(data);
		objects.push_back(temp);
	}
	else {
		temp = new Circle(data);
		objects.push_back(temp);
	}
}

void GameManager::destroyHitObject(int index){
	//declare a hitobject dead
	dead_objects.push_back(objects[index]);
	objects.erase(objects.begin()+index);
}

void GameManager::destroyDeadHitObject(int index){
	//somehow "kill" the "dead" object
	delete dead_objects[index];
	dead_objects.erase(dead_objects.begin()+index);
}

void GameManager::render_points(){
	//garbage digit finder code but it works, NO IT DOESNT WORK WITH SOME SKINS
    
}

void GameManager::render_combo(){
	//garbage digit finder code but it works, NO IT DOESNT WORK WITH SOME SKINS
    
}
