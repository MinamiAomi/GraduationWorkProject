#include "GameSystem.h"

#include "LightDeviceInput.h"

GameSystem* GameSystem::GetInstance() {
	static GameSystem instance;
	return &instance;
}

void GameSystem::OnUpdate() {
	LightDeviceInput* lightDeviceInput = LightDeviceInput::GetInstance();
	if (lightDeviceInput->GetConnectionState() == LightDeviceInput::ConnectionState::Connected &&
		playDevice_ == PlayDevice::LightDevice) {
		lightDeviceInput->Update();
	}
}

void GameSystem::OnFinalize() {
	LightDeviceInput* lightDeviceInput = LightDeviceInput::GetInstance();
	if (lightDeviceInput->GetConnectionState() == LightDeviceInput::ConnectionState::Connected) {
		lightDeviceInput->Finalize();
	}
}

bool GameSystem::IsTerminateSystem() const {
	return isTerminate_;
}

void GameSystem::Quit() {
	isTerminate_ = true;
}