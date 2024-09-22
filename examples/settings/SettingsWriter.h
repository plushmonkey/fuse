#pragma once

#include <fuse/ClientSettings.h>
#include <stdio.h>

#include <string>
#include <string_view>

struct SettingsWriter {
  bool Write(const fuse::ClientSettings& settings);

  virtual void WriteHeader(std::string_view category) = 0;
  virtual void WriteSetting(std::string_view name, std::string_view value) = 0;

  virtual bool WriteBegin() { return true; }
  virtual bool WriteEnd() { return true; }

private:
  void WriteShip(const fuse::ClientSettings& settings, int ship);
};

// Outputs the settings into a file
struct FileSettingsWriter : public SettingsWriter {
  FileSettingsWriter(std::string_view filename);

  bool WriteBegin() override;
  bool WriteEnd() override;

  void WriteHeader(std::string_view category) override;
  void WriteSetting(std::string_view name, std::string_view value) override;

  std::string filename;
  FILE* f = nullptr;
};
