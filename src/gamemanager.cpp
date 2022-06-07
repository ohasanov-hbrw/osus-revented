#include "gamemanager.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <vector>
#include <math.h>
#include <hitobject.hpp>

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
	InitAudioDevice();
	//initilize the window
	SetConfigFlags(FLAG_MSAA_4X_HINT);
	InitWindow(640*windowScale,480*windowScale,"osus?");
	//set the fps to the common number of 60
	SetTargetFPS(1440);
	//hide the cursor because we have a custom one
	HideCursor();
	//load all the textures (can also do this in load_game)
	hitCircle = LoadTexture("resources/skin/hitcircle.png");
    hitCircleOverlay = LoadTexture("resources/skin/hitcircleoverlay.png");
    approachCircle = LoadTexture("resources/skin/approachcircle.png");
    cursor = LoadTexture("resources/skin/Extra Cursors/cursor.png");
    selectCircle = LoadTexture("resources/skin/hitcircleselect.png");
    hit0 = LoadTexture("resources/skin/hit0.png");
    hit50 = LoadTexture("resources/skin/hit50.png");
    hit100 = LoadTexture("resources/skin/hit100.png");
    hit300 = LoadTexture("resources/skin/hit300.png");
    sliderb = LoadTexture("resources/skin/sliderb0.png");
    reverseArrow = LoadTexture("resources/skin/reversearrow.png");
    for(int i = 0; i < 10; i++)
    	numbers[i] = LoadTexture(("resources/skin/default-" + (std::to_string(i)) + ".png").c_str());
    timingSettingsForHitObject.sliderSpeedOverride = 1;
}

//main game loop
void GameManager::update(){
	//update the music an  d get the time from it
	UpdateMusicStream(backgroundMusic);
	currentTime = GetMusicTimePlayed(backgroundMusic);
	//get the mouse position and state
	MousePosition = Vector2{(float)GetMouseX(), (float)GetMouseY()};
	pressed = IsMouseButtonPressed(0);
	down = IsMouseButtonDown(0);
	//currently not used that much but it will be
	int timingSize = gameFile.timingPoints.size();
	for(int i = timingSize-1; i >= 0; i--){
		if(gameFile.timingPoints[i].time - gameFile.preempt <= currentTime*1000){
			time = gameFile.timingPoints[i].time;
			float tempBeatLength;
			tempBeatLength = gameFile.timingPoints[i].beatLength;
			//std::cout << "beatLength: " << tempBeatLength << std::endl;
			if(tempBeatLength > 0)
				timingSettingsForHitObject.beatLength = tempBeatLength;
			timingSettingsForHitObject.meter = gameFile.timingPoints[i].meter;
			timingSettingsForHitObject.sampleSet = gameFile.timingPoints[i].sampleSet;
			timingSettingsForHitObject.sampleIndex = gameFile.timingPoints[i].sampleIndex;
			timingSettingsForHitObject.volume = gameFile.timingPoints[i].volume;
			timingSettingsForHitObject.uninherited = gameFile.timingPoints[i].uninherited;
			timingSettingsForHitObject.effects = gameFile.timingPoints[i].effects;
			//calculate the slider speed
			if(tempBeatLength < 0){
				timingSettingsForHitObject.sliderSpeedOverride = (100 / tempBeatLength * (-1));
			}
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
			objects[objects.size()-1]->data.timing.beatLength = timingSettingsForHitObject.beatLength;
			objects[objects.size()-1]->data.timing.meter = timingSettingsForHitObject.meter;
			objects[objects.size()-1]->data.timing.sampleSet = timingSettingsForHitObject.sampleSet;
			objects[objects.size()-1]->data.timing.sampleIndex = timingSettingsForHitObject.sampleIndex;
			objects[objects.size()-1]->data.timing.volume = timingSettingsForHitObject.volume;
			objects[objects.size()-1]->data.timing.uninherited = timingSettingsForHitObject.uninherited;
			objects[objects.size()-1]->data.timing.effects = timingSettingsForHitObject.effects;
			objects[objects.size()-1]->data.timing.sliderSpeedOverride = timingSettingsForHitObject.sliderSpeedOverride;
			objects[objects.size()-1]->data.index = objects.size()-1;
			gameFile.hitObjects.pop_back();
		}
		else
			break;
	}
	//update and check collision for every hit circle
	int newSize = objects.size();
	int oldSize = objects.size();
	for(int i = 0; i < newSize; i++){
		if(std::abs(currentTime*1000 - objects[i]->data.time) <= gameFile.p50Final){
			if (pressed){
				if (objects[i]->data.type != 2){
					if (CheckCollisionPointCircle(Vector2{(float)GetMouseX(), (float)GetMouseY()},Vector2{(float)objects[i]->data.x*windowScale,(float)objects[i]->data.y*windowScale}, 56*windowScale/2) && pressed){
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
						i--;
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
				else if (objects[i]->data.type == 2){
					if(Slider* tempslider = dynamic_cast<Slider*>(objects[i]))
						if(CheckCollisionPointCircle(MousePosition,Vector2{tempslider->renderPoints[tempslider->position].x*windowScale, tempslider->renderPoints[tempslider->position].y*windowScale} ,128*windowScale/2 ) && pressed && currentTime*1000 < tempslider->data.time + gameFile.p100Final)
        					tempslider->is_hit_at_first = true;
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
	//also update the dead objects
	for(int i = 0; i < dead_objects.size(); i++){
		dead_objects[i]->data.index = i;
		dead_objects[i]->dead_update();
	}
}

//main rendering loop
void GameManager::render(){
	//set the screen for drawing
	BeginDrawing();
	//currently the background is a set color but we can change that
	ClearBackground(Color{5,0,15,255});
	//draw the fps
	DrawFPS(10, 10);
	//this is the mouse scale... i think
	float scale = 0.6f;
	//render all the objects
	for(int i = objects.size() - 1; i >= 0; i--)
		objects[i]->render();
	for(int i = dead_objects.size() - 1; i >= 0; i--)
		dead_objects[i]->dead_render();
	//render the points and the combo
	render_points();
	render_combo();
	//draw the custom mouse cursor
	DrawTextureEx(cursor, Vector2{GetMouseX()-cursor.width*windowScale/2*scale*0.5f,GetMouseY()-cursor.height*scale*windowScale/2*0.5f},0,scale*windowScale/2, WHITE);
	EndDrawing();
}

void GameManager::run(){
	//start playing the music and set the volume, it gets quite loud
	PlayMusicStream(backgroundMusic);
    SetMusicVolume(backgroundMusic, 0.2f);
	//just run the game 
	while(!WindowShouldClose()){
		update();
		render();
	}
	CloseWindow();
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
	else if(std::stoi(gameFile.configDifficulty["ApproachRate"]) < 5){
		gameFile.preempt = 1200 - 750 * (std::stoi(gameFile.configDifficulty["ApproachRate"]) - 5) / 5;
		gameFile.fade_in = 800 - 500 * (std::stoi(gameFile.configDifficulty["ApproachRate"]) - 5) / 5;
	}
	else{
		gameFile.preempt = 1200;
		gameFile.fade_in = 800;
	}
	gameFile.p300Final = gameFile.p300 - std::stoi(gameFile.configDifficulty["OverallDifficulty"]) * gameFile.p300Change;
	gameFile.p100Final = gameFile.p100 - std::stoi(gameFile.configDifficulty["OverallDifficulty"]) * gameFile.p100Change;
	gameFile.p50Final = gameFile.p50 - std::stoi(gameFile.configDifficulty["OverallDifficulty"]) * gameFile.p50Change;
	//debug, just say what the name of the music file is and load it
	std::cout << ("resources/beatmaps/" + gameFile.configGeneral["AudioFilename"]) << std::endl;
	backgroundMusic = LoadMusicStream(("resources/beatmaps/" + gameFile.configGeneral["AudioFilename"]).c_str());
	
    //these are not used right now, USE THEM
	float hpdrainrate = std::stof(gameFile.configDifficulty["HPDrainRate"]);
	float circlesize = std::stof(gameFile.configDifficulty["CircleSize"]);
	float overalldifficulty = std::stof(gameFile.configDifficulty["OverallDifficulty"]);
	//more difficulty stuff, may also be wrong
	difficultyMultiplier = ((hpdrainrate + circlesize + overalldifficulty + clip((float)gameFile.hitObjects.size() / GetMusicTimeLength(backgroundMusic) * 8.f, 0.f, 16.f)) / 38.f * 5.f);
	if (gameFile.configDifficulty.find("SliderMultiplier") != gameFile.configDifficulty.end())
		sliderSpeed = std::stof(gameFile.configDifficulty["SliderMultiplier"]);
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
		temp = new Circle(data);
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
    int digits = 1;
    int tempScore = score;
    while(!false){
    	if (tempScore < 10)
    		break;
    	digits++;
    	tempScore /= 10;
    }
    for(int i = digits; i >= 1 ; i--){
        int number = score;
        int mod = 10;
        for(int j = 1; j < i; j++)
        	mod *= 10;
        number = (number % mod - number % (mod/10))/(mod/10);
        DrawTextureEx(numbers[number], Vector2{0 + (digits - i - 1) * (numbers[0].width - 150)*windowScale/2, 0 },0,windowScale/2, Fade(WHITE, 1));
    }
}

void GameManager::render_combo(){
	//garbage digit finder code but it works, NO IT DOESNT WORK WITH SOME SKINS
    int digits = 1;
    int tempCombo = clickCombo;
    while(!false){
    	if (tempCombo < 10)
    		break;
    	digits++;
    	tempCombo /= 10;
    }
    for(int i = digits; i >= 1 ; i--){
        int number = clickCombo;
        int mod = 10;
        for(int j = 1; j < i; j++)
        	mod *= 10;
        number = (number % mod - number % (mod/10))/(mod/10);
        DrawTextureEx(numbers[number], Vector2{0 + (digits - i - 1) * (numbers[0].width - 150)*windowScale/2 /2, 440*windowScale },0,0.5f*windowScale/2, Fade(WHITE, 1));
    }
}
