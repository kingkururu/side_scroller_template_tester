#include "scenes.hpp"

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// Base scene functions  
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

// Scene constructure sets up window and sprite respawn times 
Scene::Scene( sf::RenderWindow& gameWindow ) : window(gameWindow), quadtree(0.0f, 0.0f, Constants::WORLD_WIDTH, Constants::WORLD_HEIGHT){ 
    MetaComponents::view = sf::View(Constants::VIEW_RECT); 
    log_info("scene made"); 
}

void Scene::runScene() {
    if (FlagSystem::flagEvents.gameEnd) return; // Early exit if game ended
    
    setTime();

    handleInput();

    respawnAssets();

    handleGameEvents();
    handleGameFlags();
    handleSceneFlags();

    update();
    draw();
}

void Scene::draw(){
    window.clear(sf::Color::Black);
    window.display(); 
 }

void Scene::moveViewPortWASD(){
    // move view port 
    if(FlagSystem::flagEvents.aPressed){
        MetaComponents::view.move(sf::Vector2f(-10, 0)); 
    }
    if(FlagSystem::flagEvents.dPressed){
        MetaComponents::view.move(sf::Vector2f(10, 0)); 
    }
    if(FlagSystem::flagEvents.sPressed){
        MetaComponents::view.move(sf::Vector2f(0, 10)); 
    }
    if(FlagSystem::flagEvents.wPressed){
        MetaComponents::view.move(sf::Vector2f(0, -10)); 
    }
}

// Resets everything for scene to start again. The position, moveState, flagEvents, etc are all reset 
void Scene::restartScene() {
    // re-play background music

    // set sprite movestates to true
   
    // re-set sprite and text positions 

    // clear respawn time vectors or any other unecessary vectors 

    // re-set flagEvents
    sceneEvents.resetFlags(); 
}

// Handles events from flagEvents
void Scene::handleGameFlags(){
    // if flagEvents.gameEnd is true or some event ... do somthing 
    if(FlagSystem::flagEvents.gameEnd){
       
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// Game Scene #1 down below 
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

// Gets called once before the main game loop 
void gamePlayScene::createAssets() {
    try {
        globalTimer.Reset();  

        // Sprite vectors
        std::weak_ptr<sf::Uint8[]> cloudBlueBitmaskWeakPtr = Constants::CLOUDBLUE_BITMASK;  
        cloudBlue.push_back(std::make_unique<Cloud>(Constants::CLOUDBLUE_POSITION, Constants::CLOUDBLUE_SCALE, Constants::CLOUDBLUE_TEXTURE, Constants::CLOUDBLUE_SPEED, Constants::CLOUDBLUE_ACCELERATION, cloudBlueBitmaskWeakPtr));
        
        std::weak_ptr<sf::Uint8[]> cloudPurpleBitmaskWeakPtr = Constants::CLOUDPURPLE_BITMASK;  
        cloudPurple.push_back(std::make_unique<Cloud>(Constants::CLOUDPURPLE_POSITION, Constants::CLOUDBLUE_SCALE, Constants::CLOUDPURPLE_TEXTURE, Constants::CLOUDBLUE_SPEED, Constants::CLOUDBLUE_ACCELERATION, cloudPurpleBitmaskWeakPtr));

        std::weak_ptr<sf::Uint8[]> coinBitmaskWeakPtr = Constants::COIN_BITMASK;  
        coins.push_back(std::make_unique<Coin>(Constants::COIN_POSITION, Constants::COIN_SCALE, Constants::COIN_TEXTURE, Constants::COIN_SPEED, Constants::COIN_ACCELERATION, coinBitmaskWeakPtr));

        // Background sprite
        background = std::make_unique<Background>(Constants::BACKGROUND_POSITION, Constants::BACKGROUND_SCALE, Constants::BACKGROUND_TEXTURE);
        
        // Animated sprites
        player = std::make_unique<Player>(Constants::SPRITE1_POSITION, Constants::SPRITE1_SCALE, Constants::SPRITE1_TEXTURE, Constants::SPRITE1_SPEED, Constants::SPRITE1_ACCELERATION, 
                                          Constants::SPRITE1_ANIMATIONRECTS, Constants::SPRITE1_INDEXMAX, utils::convertToWeakPtrVector(Constants::SPRITE1_BITMASK));
        player->setRects(0); 

        button1 = std::make_unique<Button>(Constants::BUTTON1_POSITION, Constants::BUTTON1_SCALE, Constants::BUTTON1_TEXTURE, 
                                   Constants::BUTTON1_ANIMATIONRECTS, Constants::BUTTON1_INDEXMAX, utils::convertToWeakPtrVector(Constants::BUTTON1_BITMASK));
        button1->setRects(0); 
        button1->setVisibleState(false); 

        // Tiles and tilemap
        for (int i = 0; i < Constants::TILES_NUMBER; ++i) {
            tiles1.at(i) = std::make_shared<Tile>(Constants::TILES_SCALE, Constants::TILES_TEXTURE, Constants::TILES_SINGLE_RECTS[i], Constants::TILES_BITMASKS[i], Constants::TILES_BOOLS[i]); 
        }
        tileMap1 = std::make_unique<TileMap>(tiles1.data(), Constants::TILES_NUMBER, Constants::TILEMAP_WIDTH, Constants::TILEMAP_HEIGHT, Constants::TILE_WIDTH, Constants::TILE_HEIGHT, Constants::TILEMAP_FILEPATH, Constants::TILEMAP_POSITION); 

        // Music and sound effects
        backgroundMusic = std::make_unique<MusicClass>(std::move(Constants::BACKGROUNDMUSIC_MUSIC), Constants::BACKGROUNDMUSIC_VOLUME);
        if(backgroundMusic) backgroundMusic->returnMusic().play(); 
        if(backgroundMusic) backgroundMusic->returnMusic().setLoop(Constants::BACKGROUNDMUSIC_LOOP);

        playerJumpSound = std::make_unique<SoundClass>(Constants::PLAYERJUMP_SOUNDBUFF, Constants::PLAYERJUMPSOUND_VOLUME); 
        coinHitSound = std::make_unique<SoundClass>(Constants::COINHIT_SOUNDBUFF, Constants::COINHITSOUND_VOLUME); 
        
        // Text
        introText = std::make_unique<TextClass>(Constants::TEXT_POSITION, Constants::TEXT_SIZE, Constants::TEXT_COLOR, Constants::TEXT_FONT, Constants::TEXT_MESSAGE);
        
        insertItemsInQuadtree(); 
        setInitialTimes();

        globalTimer.End("initializing assets in scene 1"); // for logging purposes
    } 
    catch (const std::exception& e) {
        log_error("Exception in createAssets: " + std::string(e.what()));
    }
}

void gamePlayScene::setInitialTimes(){
    cloudBlueRespawnTime = Constants::CLOUDBLUE_INITIAL_RESPAWN_TIME;
    cloudPurpleRespawnTime = Constants::CLOUDPURPLE_INITIAL_RESPAWN_TIME;
    coinRespawnTime = Constants::COIN_INITIAL_RESPAWN_TIME;
}

void gamePlayScene::insertItemsInQuadtree(){
    quadtree.insert(player);  
    quadtree.insert(button1); 

    for (auto &cloud : cloudBlue) quadtree.insert(cloud);
    for (auto &cloud : cloudPurple) quadtree.insert(cloud);
    for (auto &coin : coins) quadtree.insert(coin);
}

void gamePlayScene::respawnAssets(){
    if(cloudBlueRespawnTime <= 0 && cloudBlue.size() < Constants::CLOUDBLUE_LIMIT){
        float newCloudBlueInterval = Constants::CLOUDBLUE_INITIAL_RESPAWN_TIME - MetaComponents::globalTime * 0.38;
        std::weak_ptr<sf::Uint8[]> cloudBlueBitmaskWeakPtr = Constants::CLOUDBLUE_BITMASK;  
        cloudBlue.push_back(std::make_unique<Cloud>(Constants::makeRandomPositionCloud(), Constants::CLOUDBLUE_SCALE, Constants::CLOUDBLUE_TEXTURE, Constants::CLOUDBLUE_SPEED, Constants::CLOUDBLUE_ACCELERATION, cloudBlueBitmaskWeakPtr));
        
        cloudBlueRespawnTime = std::max(newCloudBlueInterval, Constants::CLOUDBLUE_INITIAL_RESPAWN_TIME);
    }
    if(cloudPurpleRespawnTime <= 0 && cloudPurple.size() < Constants::CLOUDPURPLE_LIMIT){
        float newCloudPurpleInterval = Constants::CLOUDPURPLE_INITIAL_RESPAWN_TIME - MetaComponents::globalTime * 0.38;
        std::weak_ptr<sf::Uint8[]> cloudPurpleBitmaskWeakPtr = Constants::CLOUDPURPLE_BITMASK;  
        cloudPurple.push_back(std::make_unique<Cloud>(Constants::makeRandomPositionCloud(), Constants::CLOUDBLUE_SCALE, Constants::CLOUDPURPLE_TEXTURE, Constants::CLOUDBLUE_SPEED, Constants::CLOUDBLUE_ACCELERATION, cloudPurpleBitmaskWeakPtr));
        
        cloudPurpleRespawnTime = std::max(newCloudPurpleInterval, Constants::CLOUDPURPLE_INITIAL_RESPAWN_TIME);
    }
    if(coinRespawnTime <= 0 && coins.size() < Constants::COIN_LIMIT){
        float newCoinInterval = Constants::COIN_INITIAL_RESPAWN_TIME - MetaComponents::globalTime * 0.38;
        std::weak_ptr<sf::Uint8[]> coinBitmaskWeakPtr = Constants::COIN_BITMASK;  
        coins.push_back(std::make_unique<Coin>(Constants::makeRandomPositionCoin(), Constants::COIN_SCALE, Constants::COIN_TEXTURE, Constants::COIN_SPEED, Constants::COIN_ACCELERATION, coinBitmaskWeakPtr));
        
        coinRespawnTime = std::max(newCoinInterval, Constants::COIN_INITIAL_RESPAWN_TIME);
    }
} 

// void gamePlayScene::respawnAssets() {
//     // Lambda to handle asset respawning logic
//     auto respawnAsset = [&](auto& assetList, float& respawnTime, float initialRespawnTime,
//                             std::function<sf::Vector2f()> makeRandomPosition, const sf::Vector2f scale, const sf::Texture texture, const sf::Vector2f speed, const sf::Vector2f acceleration, const std::weak_ptr<sf::Uint8[]>& bitmask,
//                             unsigned short limit) {
//         if (respawnTime <= 0 && assetList.size() < limit) {
//             float newInterval = initialRespawnTime - MetaComponents::globalTime * 0.38;
//             std::weak_ptr<sf::Uint8[]> ptr = bitmask;
            
//             // Assuming AssetType is the type of objects in assetList
//             using AssetType = typename std::remove_pointer<decltype(assetList)::value_type>::type;

//             assetList.push_back(std::make_unique<AssetType>(Constants::makeRandomPosition(), scale, *texture, speed, acceleration, ptr));
//             respawnTime = std::max(newInterval, initialRespawnTime);
//         }
//     };

//     // Respawn logic for each asset type
//     respawnAsset(cloudBlue, cloudBlueRespawnTime, Constants::CLOUDBLUE_INITIAL_RESPAWN_TIME,
//                  Constants::makeRandomPositionCloud, Constants::CLOUDBLUE_SCALE, Constants::CLOUDBLUE_TEXTURE, Constants::CLOUDBLUE_SPEED, Constants::CLOUDBLUE_ACCELERATION, Constants::CLOUDBLUE_BITMASK,
//                  Constants::CLOUDBLUE_LIMIT);
// }

void gamePlayScene::handleInvisibleSprites() {
    auto resetPosition = [&](auto& assetList, std::function<sf::Vector2f()> positionCallback) {
        for (auto& asset : assetList) {
           if(asset && !(*asset).getVisibleState()){
                asset->changePosition(positionCallback());
                asset->updatePos();
                asset->setVisibleState(true);
            }
        }
    };
    resetPosition(cloudBlue, Constants::makeRandomPositionCloud);
    resetPosition(cloudPurple, Constants::makeRandomPositionCloud);
    resetPosition(coins, Constants::makeRandomPositionCoin);
}

/* Updating time from GameManager's deltatime; it updates sprite respawn times and also counts 
elapsed times from when bullets were spawned and when space button was pressed */
void gamePlayScene::setTime(){
    // count respawn time here 
    if (FlagSystem::flagEvents.spacePressed || MetaComponents::spacePressedElapsedTime) {
        MetaComponents::spacePressedElapsedTime += MetaComponents::deltaTime; 
    } else {
        MetaComponents::spacePressedElapsedTime = 0.0f; 
    }
    
    cloudBlueRespawnTime -= MetaComponents::deltaTime; 
    cloudPurpleRespawnTime -= MetaComponents::deltaTime;
    coinRespawnTime -= MetaComponents::deltaTime;
} 

void gamePlayScene::handleInput() {
    handleMouseClick();
    handleSpaceKey(); 
    handleMovementKeys();
}

void gamePlayScene::handleMouseClick() {    
    if (FlagSystem::flagEvents.mouseClicked) {
        if (button1->getVisibleState() && 
            physics::collisionHelper(button1, MetaComponents::mouseClickedPosition_f)) {
            log_info("button clicked");

            button1->setClickedBool(true);
            FlagSystem::gameSceneNextFlags.sceneStart = true;
            FlagSystem::gameSceneNextFlags.sceneEnd = false;

            window.clear();
        }
    }
}

void gamePlayScene::handleSpaceKey() {
    if (MetaComponents::spacePressedElapsedTime) {
        if (player->getMoveState() && !FlagSystem::gameScene1Flags.playerFalling) {
            physics::spriteMover(player, physics::jump, MetaComponents::spacePressedElapsedTime, Constants::SPRITE1_JUMP_ACCELERATION);
        } 
    }
}

void gamePlayScene::handleMovementKeys() {
    if(!player->getMoveState()) return; 
        // move player sprite
    sf::FloatRect background1Bounds = background->getViewBounds(background->returnSpritesShape());

    // Left movement
    if (FlagSystem::flagEvents.aPressed ) physics::spriteMover(player, physics::moveLeft);
    // Right movement
    if (FlagSystem::flagEvents.dPressed) physics::spriteMover(player, physics::moveRight);
    // Down movement
    if (FlagSystem::flagEvents.sPressed) physics::spriteMover(player, physics::moveDown);
    // Up movement
    if (FlagSystem::flagEvents.wPressed) physics::spriteMover(player, physics::moveUp);
    
    if(player->getSpritePos().y > background1Bounds.height + background->getSpritePos().y){
        player->setMoveState(false);
        std::cout << "player fell off screen" << std::endl;
    } 
    else if(player->getSpritePos().y < background->getSpritePos().y){
        player->changePosition({player->getSpritePos().x, background->getSpritePos().y});
        player->updatePos();
    }
}

// Keeps sprites inside screen bounds, checks for collisions, update scores, and sets flagEvents.gameEnd to true in an event of collision 
void gamePlayScene::handleGameEvents() { 
  //  if (player) physics::spriteMover(player, physics::moveRight); 

    auto setVisibility = [&](auto& assetList) {
        for (auto& asset : assetList) {
            if (asset && asset->getSpritePos().x + 300 < MetaComponents::getViewMinX() - Constants::PASSTHROUGH_OFFSET) {
                asset->setVisibleState(false);
            }
        }
    };
    setVisibility(cloudBlue);
    setVisibility(cloudPurple);
    setVisibility(coins);
  
    for(auto it = coins.begin(); it != coins.end(); ++it){
        if(physics::collisionHelper(player, *it, physics::pixelPerfectCollision, quadtree)){
            (*it)->setVisibleState(false);
            coinHitSound->returnSound().play();
        }
    }

    bool touchingCloud = false;
    auto checkCloudCollisions = [&](const std::vector<std::unique_ptr<Cloud>>& clouds) {
        for (const auto& cloud : clouds) {
            if (player && cloud) {
                bool isTouchingThisCloud = physics::collisionHelper(player, cloud, physics::pixelPerfectCollision, quadtree);
                if (isTouchingThisCloud) {
                    touchingCloud = true;
                    break; // Stop checking further once touching a cloud
                }
            }
        }
    };

    // Check collisions for both blue and purple clouds
    checkCloudCollisions(cloudBlue);
    checkCloudCollisions(cloudPurple);

    if(MetaComponents::spacePressedElapsedTime == MetaComponents::deltaTime){
        if(player && playerJumpSound) playerJumpSound->returnSound().play();
    }

    //Update falling state based on whether the player is touching any cloud
    FlagSystem::gameScene1Flags.playerFalling = !touchingCloud;
    FlagSystem::gameScene1Flags.playerJumping = MetaComponents::spacePressedElapsedTime > 0.0f; 
    FlagSystem::gameScene1Flags.sceneEnd = !player->getMoveState();
} 

void gamePlayScene::handleSceneFlags(){
    if(FlagSystem::gameScene1Flags.playerFalling) {
        physics::spriteMover(player, physics::freeFall); 
    }
    if(FlagSystem::gameScene1Flags.sceneEnd){
        if(backgroundMusic) backgroundMusic->setVolume(Constants::BACKGROUNDMUSIC_ENDINGVOLUME); 
        button1->setVisibleState(true);
        
        button1->setPosition({player->getSpritePos().x, player->getSpritePos().y - 400});  
        button1->updatePos();
        std::cout << button1->getSpritePos().x << " " << button1->getSpritePos().y << std::endl; 
    }
}

void gamePlayScene::update() {
    try {
        updateEntityStates();
        changeAnimation();
        updateDrawablesVisibility(); 
        handleInvisibleSprites();

        updatePlayerAndView(); 
        quadtree.update(); 

        // Set the view for the window
        window.setView(MetaComponents::view);
        
    } catch (const std::exception& e) {
        log_error("Exception in updateSprites: " + std::string(e.what()));
    }
}

void gamePlayScene::updateEntityStates(){ // manually change the sprite's state
    player->setJumpingState(FlagSystem::gameScene1Flags.playerJumping);
    player->setFallingState(FlagSystem::gameScene1Flags.playerFalling); 
}

void gamePlayScene::changeAnimation(){ // change animation for sprites. change animation for texts if necessary 
    std::cout <<button1->getVisibleState() << std::endl;
    if (button1 && button1->getVisibleState()) button1->changeAnimation(); 
    if (background) background->updateBackground(Constants::BACKGROUND_SPEED, Constants::BACKGROUND_MOVING_DIRECTION);
    if (player && player->getVisibleState()) player->changeAnimation();
}

void gamePlayScene::updatePlayerAndView() {
    if(FlagSystem::gameScene1Flags.playerJumping) return;
    // Get the player's position
    sf::Vector2f playerPos = player->getSpritePos();

    // Calculate the view size and game world boundaries
    sf::Vector2f viewSize = MetaComponents::view.getSize();
    sf::FloatRect worldBounds(0, 0, Constants::WORLD_WIDTH, Constants::WORLD_HEIGHT);

    // Offset at the bottom of the background to consider any gaps or padding
    const float bottomOffset = 330.0;

    // Default view center is the player's position
    sf::Vector2f viewCenter = playerPos;

    // Lock the view within the top and bottom boundaries
    float minViewY = viewSize.y / 2.0f;
    float maxViewY = std::max(worldBounds.height - viewSize.y / 2.0f - bottomOffset, minViewY);

    // Clamp the view center's Y-coordinate within valid bounds
    viewCenter.y = std::clamp(viewCenter.y, minViewY, maxViewY);

    // Update the view center
    MetaComponents::view.setCenter(viewCenter);
}

void gamePlayScene::updateDrawablesVisibility(){
    try{
       // if (button1 && button1->getVisibleState()) button1->setVisibleState(physics::collisionHelper(button1, MetaComponents::view)); // reset button's visibility if it is on or off screen 
    }
    catch(const std::exception & e){
        log_error("Exception in updateDrawablesVisibility: " + std::string(e.what()));
    }
}

// Draws only the visible sprite and texts
void gamePlayScene::draw() {
    try {
        window.clear(sf::Color::Blue); // set the base baskground color blue

        auto drawAnythingVisible = [&](auto& drawable) {
            if (drawable && drawable->getVisibleState()) window.draw(*drawable);
        };
    
        drawAnythingVisible(background);
        drawAnythingVisible(tileMap1);
        drawAnythingVisible(introText);
        drawAnythingVisible(button1);
        for (auto& cloud : cloudBlue) drawAnythingVisible(cloud);
        for (auto& cloud : cloudPurple) drawAnythingVisible(cloud);
        for (auto& coin : coins) drawAnythingVisible(coin);
        drawAnythingVisible(player);

        window.display(); 
    } 
    
    catch (const std::exception& e) {
        log_error("Exception in draw: " + std::string(e.what()));
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// Game Scene #2 from down below 
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

void gamePlayScene2::createAssets() {
 try {
        // Initialize sprites and music here 
        background = std::make_unique<Background>(Constants::BACKGROUND_POSITION, Constants::BACKGROUND_SCALE, Constants::BACKGROUND_TEXTURE2);
    } 

    catch (const std::exception& e) {
        log_error("Exception in createAssets: " + std::string(e.what()));
    }
}

void gamePlayScene2::handleInput() {
    moveViewPortWASD(); // change position of the view port based on keyboard input flags
}

void gamePlayScene2::draw() {
    try {
        window.clear(); // clear elements from previous screen 

        if (background && background->getVisibleState()) window.draw(*background); 
    
        window.display(); 
    } 
    
    catch (const std::exception& e) {
        log_error("Exception in gamePlayScene2 draw: " + std::string(e.what()));
    }
}

void gamePlayScene2::update() {
    try {
        handleInvisibleSprites(); // do a sprite pooling or actually delete all

        window.setView(MetaComponents::view); 
        
    }
    catch (const std::exception& e) {
        log_error("Exception in updateSprites: " + std::string(e.what()));
    }
}
