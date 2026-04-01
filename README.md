<div align="center">
  <img src="[https://upload.wikimedia.org/wikipedia/commons/9/91/Octicons-mark-github.svg](https://upload.wikimedia.org/wikipedia/commons/9/91/Octicons-mark-github.svg)" alt="Logo" width="80" height="80">
  <h3 align="center">GitHub Explorer</h3>
  <p align="center">
    A lightning-fast, native desktop client for exploring GitHub.
    <br />
    <a href="[https://github.com/YOUR_USERNAME/github-explorer-qt/releases/latest](https://github.com/YOUR_USERNAME/github-explorer-qt/releases/latest)"><strong>Download for Windows »</strong></a>
  </p>
</div>

<div align="center">

![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg?style=for-the-badge&logo=c%2B%2B)
![Qt](https://img.shields.io/badge/Qt-6.7-41CD52.svg?style=for-the-badge&logo=qt)
![License](https://img.shields.io/badge/License-MIT-green.svg?style=for-the-badge)

</div>

## 🌌 About The Project

![Hero Screenshot](screenshots/dark-2.png)

GitHub Explorer is a premium, native desktop application designed to interact with the GitHub REST API. Built entirely in C++ and Qt, it aims to provide a fast, memory-efficient, and visually stunning alternative to web-based exploration. 

### ✨ Key Features
* **Lightning Fast Search:** Instantly find developers, view their stats, and explore their repositories.
* **Explore Dashboard:** Discover trending projects, most-starred repositories, most-forked projects, and top developers with advanced time and limit filters.
* **Native Bookmarking:** Save your favorite repositories locally using a robust SQLite/Settings backend.
* **Premium UI/UX:** Features a fully responsive, master-detail layout with fluid animations and dynamic Light/Dark mode toggling.
* **API Authentication:** Support for GitHub Personal Access Tokens to securely increase rate limits from 60 to 5,000 requests per hour.

## 🛠️ Built With
* **C++17** - Core application logic and memory management.
* **Qt 6.7** - UI framework, networking (`QNetworkAccessManager`), and concurrent processing.
* **Qt Style Sheets (QSS)** - Completely custom, CSS-driven UI components.
* **Lucide Icons** - Scalable, theme-aware vector graphics.

## 🚀 Getting Started

### Prerequisites
To compile this project from source, you will need:
* Qt 6.5 or higher (with the Qt Network module)
* CMake or qmake
* A C++17 compatible compiler (MinGW / MSVC / Clang)

### Installation
1. Clone the repo:
    git clone [https://github.com/sekhar-dev79/github-explorer-qt.git](https://github.com/sekhar-dev79/github-explorer-qt.git)

2. Open `GitHubExplorer.pro` in Qt Creator.
3. Build in Release mode and Run!

## 📸 Showcase

Experience a premium native UI with seamless switching between Light and Dark modes.

| Light Theme | Dark Theme |
| :---: | :---: |
| **Search & Developer Profiles** |
| ![Search Light](screenshots/light-1.png) | ![Search Dark](screenshots/dark-1.png) |
| **Explore Dashboard (Trending & Top)** |
| ![Explore Light](screenshots/light-2.png) | ![Explore Dark](screenshots/dark-2.png) |
| **Repository Details & Stats** |
| ![Repos Light](screenshots/light-3.png) | ![Repos Dark](screenshots/dark-3.png) |
| **Local Bookmarks** |
| ![Bookmarks Light](screenshots/light-4.png) | ![Bookmarks Dark](screenshots/dark-4.png) |
| **Settings & API Authentication** |
| ![Settings Light](screenshots/light-5.png) | ![Settings Dark](screenshots/dark-5.png) |

## 📄 License
Distributed under the MIT License. See `LICENSE` for more information.
