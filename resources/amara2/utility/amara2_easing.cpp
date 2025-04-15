namespace Amara {
    enum class Ease {
        Linear,
        SineIn,
        SineOut,
        SineInOut,

        CubicIn,
        CubicOut,
        CubicInOut,

        BackIn,
        BackOut,
        BackInOut,

        BounceIn,
        BounceOut,
        BounceInOut,

        ElasticIn,
        ElasticOut,
        ElasticInOut,

        CircIn,
        CircOut,
        CircInOut,

        ExpoIn,
        ExpoOut,
        ExpoInOut
    };

    double easeBounceOut(double x) {
        const double n1 = 7.5625;
        const double d1 = 2.75;

        if (x < 1 / d1) {
            return n1 * x * x;
        } else if (x < 2 / d1) {
            return n1 * (x -= 1.5 / d1) * x + 0.75;
        } else if (x < 2.5 / d1) {
            return n1 * (x -= 2.25 / d1) * x + 0.9375;
        } else {
            return n1 * (x -= 2.625 / d1) * x + 0.984375;
        }
    }

    // Progress is between 0 and 1.
    double ease(double startVal, double endVal, double progress, Amara::Ease easing) {
        double eased_progress = 0;
        switch (easing) {
            case Ease::SineIn:
                eased_progress = 1 - cos((progress * M_PI) / 2);
                break;
             case Ease::SineOut:
                eased_progress = sin((progress * M_PI) / 2);
                break;
            case Ease::SineInOut:
                eased_progress = -(cos(M_PI * progress) - 1) / 2;
                break;
            case Ease::CubicIn:
                eased_progress = progress * progress * progress;
                break;
            case Ease::CubicOut:
                eased_progress = (progress - 1) * (progress - 1) * (progress - 1) + 1;
                break;
            case Ease::CubicInOut:
                eased_progress = progress < 0.5
                    ? 4 * progress * progress * progress
                    : (progress - 1) * (2 * progress - 2) * (2 * progress - 2) + 1;
                break;
            case Ease::BackIn: {
                const double c1 = 1.70158;
                const double c3 = c1 + 1;
                eased_progress = c3 * progress * progress * progress - c1 * progress * progress;
                break;
            }
            case Ease::BackOut: {
                const double c1 = 1.70158;
                const double c3 = c1 + 1;
                eased_progress = 1 + c3 * pow(progress - 1, 3) + c1 * pow(progress - 1, 2);
                break;
            }
            case Ease::BackInOut: {
                const double c1 = 1.70158;
                const double c2 = c1 * 1.525;

                eased_progress = progress < 0.5
                    ? (pow(2 * progress, 2) * ((c2 + 1) * 2 * progress - c2)) / 2
                    : (pow(2 * progress - 2, 2) * ((c2 + 1) * (progress * 2 - 2) + c2) + 2) / 2;
                break;
            }
            case Ease::BounceIn:
                eased_progress = 1 - easeBounceOut(1 - progress);
                break;
            case Ease::BounceOut:
                eased_progress = easeBounceOut(progress);
                break;
            case Ease::BounceInOut:
                eased_progress = progress < 0.5
                    ? (1 - easeBounceOut(1 - 2 * progress)) / 2
                    : (1 + easeBounceOut(2 * progress - 1)) / 2;
                break;
            case Ease::ElasticIn: {
                const double c4 = (2 * M_PI) / 3;
                eased_progress = progress == 0
                    ? 0
                    : progress == 1
                    ? 1
                    : -pow(2, 10 * progress - 10) * sin((progress * 10 - 10.75) * c4);
                break;
            }
            case Ease::ElasticOut: {
                const double c4 = (2 * M_PI) / 3;
                eased_progress = progress == 0
                    ? 0
                    : progress == 1
                    ? 1
                    : pow(2, -10 * progress) * sin((progress * 10 - 0.75) * c4) + 1;
                break;
            }
            case Ease::ElasticInOut: {
                const double c5 = (2 * M_PI) / 4.5;
                eased_progress = progress == 0
                    ? 0
                    : progress == 1
                    ? 1
                    : progress < 0.5
                    ? -(pow(2, 20 * progress - 10) * sin((20 * progress - 11.125) * c5)) / 2
                    : (pow(2, -20 * progress + 10) * sin((20 * progress - 11.125) * c5)) / 2 + 1;
                break;
            }
            case Ease::CircIn:
                eased_progress = 1 - sqrt(1 - pow(progress, 2));
                break;
            case Ease::CircOut:
                eased_progress = sqrt(1 - pow(progress - 1, 2));
                break;
            case Ease::CircInOut:
                eased_progress = progress < 0.5
                    ? (1 - sqrt(1 - pow(2 * progress, 2))) / 2
                    : (sqrt(1 - pow(-2 * progress + 2, 2)) + 1) / 2;
                break;
            case Ease::ExpoIn:
                eased_progress = progress == 0 ? 0 : pow(2, 10 * progress - 10);
                break;
            case Ease::ExpoOut:
                eased_progress = progress == 1 ? 1 : 1 - pow(2, -10 * progress);
                break;
            case Ease::ExpoInOut:
                eased_progress = progress == 0
                    ? 0
                    : progress == 1
                    ? 1
                    : progress < 0.5
                    ? pow(2, 20 * progress - 10) / 2
                    : (2 - pow(2, -20 * progress + 10)) / 2;
                break;
            case Ease::Linear:
            default:
                eased_progress = progress;
                break;
        }
        return (endVal - startVal)*eased_progress + startVal;
    }
    double ease(double startVal, double endVal, double progress) {
        return ease(startVal, endVal, progress, Ease::Linear);
    }

    Amara::Color ease(const Amara::Color& startColor, const Amara::Color& endColor, double progress, Amara::Ease easing) {
        return Amara::Color(
            static_cast<Uint8>(ease(startColor.r, endColor.r, progress, easing)),
            static_cast<Uint8>(ease(startColor.g, endColor.g, progress, easing)),
            static_cast<Uint8>(ease(startColor.b, endColor.b, progress, easing)),
            static_cast<Uint8>(ease(startColor.a, endColor.a, progress, easing))
        );
    }
    Amara::Color ease(const Amara::Color& startColor, const Amara::Color& endColor, double progress) {
        return ease(startColor, endColor, progress, Ease::Linear);
    }

    void bindLua_Easing(sol::state& lua) {
        lua.new_enum("Ease",
            "Linear", Ease::Linear,
            "SineIn", Ease::SineIn,
            "SineOut", Ease::SineOut,
            "SineInOut", Ease::SineInOut,

            "CubicIn", Ease::CubicIn,
            "CubicOut", Ease::CubicOut,
            "CubicInOut", Ease::CubicInOut,

            "BackIn", Ease::BackIn,
            "BackOut", Ease::BackOut,
            "BackInOut", Ease::BackInOut,

            "BounceIn", Ease::BounceIn,
            "BounceOut", Ease::BounceOut,
            "BounceInOut", Ease::BounceInOut,

            "ElasticIn", Ease::ElasticIn,
            "ElasticOut", Ease::ElasticOut,
            "ElasticInOut", Ease::ElasticInOut,

            "CircIn", Ease::CircIn,
            "CircOut", Ease::CircOut,
            "CircInOut", Ease::CircInOut,

            "ExpoIn", Ease::ExpoIn,
            "ExpoOut", Ease::ExpoOut,
            "ExpoInOut", Ease::ExpoInOut
        );
        lua.set_function("ease", sol::overload(
            sol::resolve<double(double, double, double, Amara::Ease)>(&Amara::ease),
            sol::resolve<double(double, double, double)>(&Amara::ease)
        ));
    }
}
