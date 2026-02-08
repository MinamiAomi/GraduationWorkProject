#pragma once

class LevelManager {
public:

    static LevelManager* GetInstance() {
        static LevelManager instance;
        return &instance;
    }

    LevelManager(const LevelManager&) = delete;
    LevelManager& operator=(const LevelManager&) = delete;


    enum class Level {
        LEVEL1,
        LEVEL2
    };

    void SetLevel(const Level& level) { selectLevel_ = level; }
    const Level& GetLevel() const { return selectLevel_; } 

private:
    LevelManager() : selectLevel_(Level::LEVEL1) {}
    ~LevelManager() = default;

    Level selectLevel_;
};