#ifndef TIMER_H
#define TIMER_H

#include <chrono>

typedef std::chrono::time_point<std::chrono::steady_clock> Clock;

class Timer
{
public:
	enum RESOLUTION
	{
		RESOLUTION_HOURS,
		RESOLUTION_MINUTES,
		RESOLUTION_SECONDS,
		RESOLUTION_MILLISECONDS,
		RESOLUTION_MICROSECONDS,
		RESOLUTION_NANOSECONDS
	};

	void SetResolution(RESOLUTION _newResolution)
	{
		switch (_newResolution)
		{
		case Timer::RESOLUTION_HOURS:
			timeResolution = ResolutionHours;
			resolutionMultiplier = 3600.0;
			break;
		case Timer::RESOLUTION_MINUTES:
			timeResolution = ResolutionMinutes;
			resolutionMultiplier = 60.0;
			break;
		case Timer::RESOLUTION_SECONDS:
			timeResolution = ResolutionSeconds;
			resolutionMultiplier = 1.0;
			break;
		case Timer::RESOLUTION_MILLISECONDS:
			timeResolution = ResolutionMilliseconds;
			resolutionMultiplier = 0.001;
			break;
		case Timer::RESOLUTION_MICROSECONDS:
			timeResolution = ResolutionMicroseconds;
			resolutionMultiplier = 0.000001;
			break;
		case Timer::RESOLUTION_NANOSECONDS:
			timeResolution = ResolutionNanoseconds;
			resolutionMultiplier = 0.000000001;
			break;
		}
	}

	void Play()
	{
		paused = false;
		clock = std::chrono::high_resolution_clock::now();
	}
	void Pause()
	{
		UpdateTime();
		paused = true;
	}
	void Reset()
	{
		time = 0.0;
		clock = std::chrono::high_resolution_clock::now();
	}
	void UpdateTime()
	{
		if (paused == false)
		{
			long long clockTime = timeResolution(clock);
			time += clockTime * resolutionMultiplier;

			if (clockTime != 0)
				clock = std::chrono::high_resolution_clock::now();
		}
	}
	double GetTime()
	{
		UpdateTime();
		return time;
	}
	void SetTime(double _newTime)
	{
		time = _newTime;
	}
	bool IsPaused()
	{
		return paused;
	}

private:
	static inline long long ResolutionHours(Clock _clock)
	{
		return std::chrono::duration_cast<std::chrono::hours>(std::chrono::high_resolution_clock::now() - _clock).count();
	}
	static inline long long ResolutionMinutes(Clock _clock)
	{
		return std::chrono::duration_cast<std::chrono::minutes>(std::chrono::high_resolution_clock::now() - _clock).count();
	}
	static inline long long ResolutionSeconds(Clock _clock)
	{
		return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - _clock).count();
	}
	static inline long long ResolutionMilliseconds(Clock _clock)
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - _clock).count();
	}
	static inline long long ResolutionMicroseconds(Clock _clock)
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - _clock).count();
	}
	static inline long long ResolutionNanoseconds(Clock _clock)
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - _clock).count();
	}

	Clock clock;
	long long(*timeResolution)(Clock _clock) = ResolutionMilliseconds;
	double resolutionMultiplier = 0.001;

	double time = 0.0;

	bool paused = true;
};

#endif