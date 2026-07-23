2026/07/23
- showSettingsDialog() now calls dlg.setAutosaveInterval(m_autosaveInterval), so opening Settings no longer resets the interval to 60.
- Constructor restarts m_autosaveTimer after loadSettings(), so the saved interval takes effect at startup.
- toggleVisibility() now returns when m_isSliding, actually blocking re-entrant animations.
- Constructor calls animateSlide(true) when m_startVisible, so start-visible docks the window on-screen instead of leaving it off-screen.
- Null-screen fallback (primaryScreen(), then bail) added in both applyGeometryAndPosition() and animateSlide().
- Standard shortcuts guarded behind #ifndef Q_OS_MAC (menu owns them on Mac) to prevent unintentional overloading.
- Save path for each tab in saveLastSession; restore into m_filePaths in loadLastSession.
- Prevents crash if there's a null widget for close tab.
- Iterate version.