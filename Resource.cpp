#include "Resource.h"



IMAGE* ResourceManager::getRawImage(ImageResourceId id)
{
    Image2D* image = getImage2D(id);

    if (image == NULL)
    {
        return NULL;
    }

    return image->getImage();
}

void ResourceManager::initImageResourceTable()
{
    imagePaths.clear();

    imagePaths[IMG_BG_SKY] = _T("assets\\tex\\maps\\bg1\\Sky_1920x1080.png");
    imagePaths[IMG_BG_CLOUDS] = _T("assets\\tex\\maps\\bg1\\Clouds_1920x1080.png");
    imagePaths[IMG_BG_FLORA1] = _T("assets\\tex\\maps\\bg1\\Flora1_1920x1080.png");
    imagePaths[IMG_BG_FLORA2] = _T("assets\\tex\\maps\\bg1\\Flora2_1920x1080.png");

    imagePaths[IMG_TILESET_MAIN] = _T("assets\\tex\\maps\\tileset.png");

    imagePaths[IMG_PLAYER_IDLE_L] = _T("assets\\tex\\entities\\characters\\player1_idle_L.png");
    imagePaths[IMG_PLAYER_IDLE_R] = _T("assets\\tex\\entities\\characters\\player1_idle_R.png");
    imagePaths[IMG_PLAYER_WALK_L] = _T("assets\\tex\\entities\\characters\\player1_walk_L.png");
    imagePaths[IMG_PLAYER_WALK_R] = _T("assets\\tex\\entities\\characters\\player1_walk_R.png");
    imagePaths[IMG_PLAYER_RUN_L] = _T("assets\\tex\\entities\\characters\\player1_run_L.png");
    imagePaths[IMG_PLAYER_RUN_R] = _T("assets\\tex\\entities\\characters\\player1_run_R.png");
    imagePaths[IMG_PLAYER_JUMP_START_L] = _T("assets\\tex\\entities\\characters\\player1_jumpStart_L.png");
    imagePaths[IMG_PLAYER_JUMP_START_R] = _T("assets\\tex\\entities\\characters\\player1_jumpStart_R.png");
    imagePaths[IMG_PLAYER_JUMP_LOOP_L] = _T("assets\\tex\\entities\\characters\\player1_jumpLoop_L.png");
    imagePaths[IMG_PLAYER_JUMP_LOOP_R] = _T("assets\\tex\\entities\\characters\\player1_jumpLoop_R.png");
    imagePaths[IMG_PLAYER_JUMP_END_L] = _T("assets\\tex\\entities\\characters\\player1_jumpEnd_L.png");
    imagePaths[IMG_PLAYER_JUMP_END_R] = _T("assets\\tex\\entities\\characters\\player1_jumpEnd_R.png");

    imagePaths[IMG_PLAYER2_STATIC] = _T("assets\\tex\\entities\\characters\\player2.png");
    imagePaths[IMG_PLAYER3_STATIC] = _T("assets\\tex\\entities\\characters\\player3.png");
    imagePaths[IMG_PLAYER4_STATIC] = _T("assets\\tex\\entities\\characters\\player4.png");

    imagePaths[IMG_COIN_GOLD] = _T("assets\\tex\\entities\\items\\MonedaD.png");
    imagePaths[IMG_COIN_SILVER] = _T("assets\\tex\\entities\\items\\MonedaP.png");
    imagePaths[IMG_COIN_COPPER] = _T("assets\\tex\\entities\\items\\MonedaR.png");

    imagePaths[IMG_COIN_COLLECTED] = _T("assets\\tex\\entities\\items\\Collected.png");

    imagePaths[IMG_CHECKPOINT_NO_FLAG] = _T("assets\\tex\\entities\\items\\Checkpoint (No Flag).png");
    imagePaths[IMG_CHECKPOINT_FLAG_OUT] = _T("assets\\tex\\entities\\items\\Checkpoint (Flag Out) (64x64).png");
    imagePaths[IMG_CHECKPOINT_FLAG_IDLE] = _T("assets\\tex\\entities\\items\\Checkpoint (Flag Idle)(64x64).png");
}

void ResourceManager::loadImage2D(ImageResourceId id)
{
    if (imagePaths.find(id) == imagePaths.end())
    {
        return;
    }

    unique_ptr<Image2D> image(new Image2D());

    if (!image->load(imagePaths[id]))
    {
        return;
    }

    images[id] = move(image);
}

void ResourceManager::loadImageResources()
{
    images.clear();

    for (map<ImageResourceId, const TCHAR*>::iterator it = imagePaths.begin(); it != imagePaths.end(); ++it)
    {
        loadImage2D(it->first);
    }
}

Image2D* ResourceManager::getImage2D(ImageResourceId id)
{
    if (images.find(id) == images.end())
    {
        return NULL;
    }

    return images[id].get();
}

void ResourceManager::initTextResourceTable()
{
    textPaths.clear();
    textPaths[TXT_MAP_MAIN] = _T("assets\\tex\\maps\\map.txt");
}

void ResourceManager::loadTextFile(TextResourceId id)
{
    if (textPaths.find(id) == textPaths.end())
    {
        return;
    }

    ifstream inFile(textPaths[id]);
    if (!inFile.is_open())
    {
        cout << "Failed to open text resource file." << endl;
        return;
    }

    string content = "";
    string line;
    while (getline(inFile, line))
    {
        content += line + "\n";
    }
    textContents[id] = content;
    inFile.close();
}

void ResourceManager::loadTextResources()
{
    textContents.clear();
    for (map<TextResourceId, const TCHAR*>::iterator it = textPaths.begin(); it != textPaths.end(); ++it)
    {
        loadTextFile(it->first);
    }
}

const string& ResourceManager::getTextContent(TextResourceId id) const
{
    static const string emptyString = "";
    map<TextResourceId, string>::const_iterator it = textContents.find(id);
    if (it == textContents.end())
    {
        return emptyString;
    }
    return it->second;
}

void ResourceManager::loadLevelResources()
{
    initImageResourceTable();
    loadImageResources();

    initTextResourceTable();
    loadTextResources();
}