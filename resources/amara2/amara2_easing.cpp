namespace Amara {
    enum class Ease {
        Linear,
        SineIn,
        SineOut,
        SineInOut
    };

    // Progress is between 0 and 1.

    double ease_sineIn(double progress) {
        return 1 - cos((progress * M_PI) / 2);
    }

    double ease_sineOut(double progress) {
        return sin((progress * M_PI) / 2);
    }

    double ease_sineInOut(double progress) {
        return -(cos(M_PI * progress) - 1) / 2;
    }

    double ease(float startVal, float endVal, double progress, Amara::Ease easing) {
        double eased_progress = progress;
        switch (easing) {
            case Ease::SineIn:
                eased_progress = ease_sineIn(progress);
                break;
             case Ease::SineOut:
                eased_progress = ease_sineOut(progress);
                break;
            case Ease::SineInOut:
                eased_progress = ease_sineInOut(progress);
                break;
        }
        return (endVal - startVal)*eased_progress + startVal;
    }
    double ease(float startVal, float endVal, double progress) {
        return ease(startVal, endVal, progress, Ease::Linear);
    }

    SDL_Color ease(SDL_Color startColor, SDL_Color endColor, double progress, Amara::Ease easing) {
        return {
            ease(startColor.r, endColor.r, progress, easing),
            ease(startColor.g, endColor.g, progress, easing),
            ease(startColor.b, endColor.b, progress, easing),
            ease(startColor.a, endColor.a, progress, easing)
        };
    }
    SDL_Color ease(SDL_Color startColor, SDL_Color endColor, double progress) {
        return ease(startColor, endColor, progress, Ease::Linear);
    }

    void bindLua_Easing(sol::state& lua) {
        lua.new_enum("Ease",
            "Linear", Ease::Linear,
            "SineIn", Ease::SineIn,
            "SineOut", Ease::SineOut,
            "SineInOut", Ease::SineInOut
        );
        lua.set_function("ease", sol::overload(
            sol::resolve<double(float, float, double, Amara::Ease)>(&Amara::ease),
            sol::resolve<double(float, float, double)>(&Amara::ease)
        ));
    }
}