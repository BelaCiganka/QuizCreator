#pragma once

struct UserResult {
    int id = -1;
    int userId = -1;
    int quizId = -1;
    int score = 0;
    int total = 0;

    // started_at, finished_at, duration_ms in base; you can later add QString/chrono
};
