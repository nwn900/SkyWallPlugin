#pragma once

namespace RE
{
    struct bhkCharacterController;
}

namespace WP::Hooks
{
    class PhysicsHook
    {
    public:
        static PhysicsHook& Get();

        void Install();
        bool IsInstalled() const { return _installed; }

    private:
        PhysicsHook() = default;
        ~PhysicsHook() = default;

        bool _installed = false;
        RE::bhkCharacterController* _playerController = nullptr;
    };
}
