# Social Media Analytics Dashboard

A Qt-based desktop application for managing, importing, analyzing, visualizing, and exporting multi-platform social media post data.

The system is designed around a layered C++ architecture and a MySQL backend. It supports user authentication, role-based permissions, CSV data import, dashboard visualization, statistical analysis, report export, operation logging, and local export settings.

---

## Table of Contents

- [Project Overview](#project-overview)
- [Key Features](#key-features)
- [Technology Stack](#technology-stack)
- [Project Structure](#project-structure)
- [System Architecture](#system-architecture)
- [Database Design](#database-design)
- [Environment Requirements](#environment-requirements)
- [Installation and Setup](#installation-and-setup)
- [Usage Guide](#usage-guide)
- [CSV Import Format](#csv-import-format)
- [Permission Rules](#permission-rules)
- [Report Export](#report-export)
- [Common Issues](#common-issues)
- [Future Improvements](#future-improvements)
- [Team Members](#team-members)
- [License](#license)

---

## Project Overview

**Social Media Analytics Dashboard** is a desktop data management and analysis system built with **C++**, **Qt**, and **MySQL**.

It helps users collect and manage social media post records from platforms such as Weibo, Douyin, Bilibili, Xiaohongshu, and WeChat. The system provides visual dashboards, statistical analysis, CSV import, report export, user management, and operation logs.

The project follows a clear layered design:

```text
View
  ↓
Controller
  ↓
Service
  ↓
Repository
  ↓
DatabaseManager / MySQL
```

This structure keeps interface display, business logic, data access, and database initialization separated, making the code easier to maintain and extend.

---

## Key Features

### User Authentication

- User login with username and password.
- Automatic creation of the default administrator account.
- Account status checking for active and disabled users.
- SHA-256 password verification.
- Compatibility handling for legacy DES-encrypted passwords.

Default administrator account:

| Username | Password |
|---|---|
| `admin` | `123456` |

> For security reasons, change the default password after the first successful login.

### Role-Based User Management

- Administrator and normal user roles.
- Admin users can create new users.
- Admin users can enable or disable accounts.
- Admin users can reset user passwords to the default value.
- The system prevents disabling the currently logged-in user.
- The system prevents disabling the last active administrator.

### Post Data Management

- Add social media post records manually.
- Update selected post records.
- Delete selected post records.
- Search by content or account name.
- Filter records by platform.
- Automatically bind newly added or imported records to the current user.

### CSV Import

- Import standard CSV post records.
- Import selected platform-specific CSV exports.
- Detect invalid CSV files and unsupported formats.
- Reject Excel workbook files that are incorrectly renamed as CSV.
- Support UTF-8, UTF-8 BOM, UTF-16, and common local encodings where applicable.
- Store imported records under the current logged-in user.

### Dashboard Visualization

- Summary cards for total posts, interactions, views, and engagement rate.
- Platform post share chart.
- Platform interaction chart.
- Recent 14-day interaction trend.
- Top 5 popular posts.

### Data Analytics

- Filter analytics by platform and date range.
- Display summary statistics.
- Display platform-level statistics.
- Display date trend statistics.
- Display top posts ranked by interactions.

### Report Export

- Export reports in CSV, TXT, and HTML formats.
- Preview generated reports before saving.
- Save reports to a configurable export directory.
- Include summary statistics, platform statistics, and top posts.

### Operation Logs

- Record important system actions.
- Track login results, user management operations, post operations, CSV import, and report export.
- Search logs by username, action type, and date range.

### System Settings

- Configure the default export directory.
- Store local settings with `QSettings`.
- Display project version, build date, and project description.

---

## Technology Stack

| Category | Technology |
|---|---|
| Programming Language | C++17 |
| GUI Framework | Qt Widgets |
| Database Access | Qt SQL |
| Chart Visualization | Qt Charts |
| Database | MySQL |
| Build System | CMake |
| UI Design | Qt Designer `.ui` files |
| Local Configuration | QSettings |
| Password Hashing | SHA-256 |
| Legacy Compatibility | DES utility for old data handling |

---

## Project Structure

```text
SocialMediaAnalyticsDashboard/
├── CMakeLists.txt
├── main.cpp
├── controllers/
├── forms/
├── infrastructure/
├── models/
├── repositories/
├── services/
├── styles/
├── tools/
├── utils/
└── views/
```

### Directory Description

| Directory / File | Description |
|---|---|
| `CMakeLists.txt` | CMake build configuration. It enables Qt auto UI, MOC, and RCC handling. |
| `main.cpp` | Application entry point. It initializes the database, creates default tables, creates the default admin account, and opens the login window. |
| `controllers/` | Handles UI events, connects views with services, applies permission rules, and writes operation logs. |
| `forms/` | Qt Designer UI files for login, dashboard, post management, analytics, export, logs, settings, and user management pages. |
| `infrastructure/` | Database connection and initialization logic. |
| `models/` | Data models and view models used by users, posts, analytics, dashboard, logs, and export modules. |
| `repositories/` | Data access layer. It executes SQL queries and maps database records to model objects. |
| `services/` | Business logic layer for authentication, users, posts, CSV import, analytics, dashboard, export, logs, reports, and settings. |
| `styles/` | Application-level style definitions. |
| `tools/` | Utility scripts, such as effective line counting. |
| `utils/` | Shared utility classes and constants, including password hashing, DES compatibility, and platform constants. |
| `views/` | Qt widget pages and UI display logic. |

---

## System Architecture

The project uses a layered architecture to reduce coupling between UI code and business logic.

| Layer | Responsibility |
|---|---|
| View | Displays UI components, receives user input, and emits signals. |
| Controller | Handles view events, prepares request objects, applies permission scope, calls services, and updates views. |
| Service | Contains core business rules, validation, data isolation, report generation, and settings logic. |
| Repository | Executes SQL operations and converts database results into model objects. |
| Model | Defines structured data used by the application. |
| Infrastructure | Manages MySQL connection, table creation, database upgrade checks, and error reporting. |

---

## Database Design

The application connects to a MySQL database named `social_media_system`.

Database connection settings are defined in:

```text
infrastructure/databasemanager.cpp
```

Default local configuration:

| Item | Value |
|---|---|
| Host | `127.0.0.1` |
| Port | `3306` |
| Database | `social_media_system` |
| Username | `root` |
| Password | `123456` |

### `users` Table

Stores system users and account status.

| Field | Type | Description |
|---|---|---|
| `user_id` | `INT` | Primary key, auto-increment user ID. |
| `username` | `VARCHAR(255)` | Unique username. Stored in encrypted form. |
| `password` | `VARCHAR(255)` | User password hash. |
| `role` | `VARCHAR(20)` | User role, usually `admin` or `user`. |
| `status` | `VARCHAR(20)` | Account status, usually `active` or `disabled`. |
| `created_at` | `TIMESTAMP` | User creation time. |

### `posts` Table

Stores social media post records.

| Field | Type | Description |
|---|---|---|
| `post_id` | `INT` | Primary key, auto-increment post ID. |
| `platform` | `VARCHAR(50)` | Social media platform name. |
| `account_name` | `VARCHAR(100)` | Account or author name. |
| `content` | `TEXT` | Post title, content, or summary. |
| `publish_date` | `DATE` | Post publish date. |
| `likes` | `INT` | Number of likes. |
| `comments` | `INT` | Number of comments. |
| `shares` | `INT` | Number of shares. |
| `views` | `INT` | Number of views, reads, or plays. |
| `created_by_user_id` | `INT` | User ID of the creator or importer. |
| `created_by_username` | `VARCHAR(255)` | Username of the creator or importer. |
| `created_at` | `TIMESTAMP` | Record creation time. |

### `operation_logs` Table

Stores important user and system operations.

| Field | Type | Description |
|---|---|---|
| `log_id` | `INT` | Primary key, auto-increment log ID. |
| `user_id` | `INT` | User ID related to the operation. |
| `username` | `VARCHAR(255)` | Username related to the operation. |
| `action` | `VARCHAR(50)` | Operation type. |
| `detail` | `TEXT` | Detailed operation description. |
| `result` | `VARCHAR(20)` | Operation result, such as `success` or `failed`. |
| `created_at` | `TIMESTAMP` | Log creation time. |

---

## Environment Requirements

Before building and running the project, make sure the following components are installed:

| Requirement | Recommended Version / Note |
|---|---|
| C++ Compiler | A compiler that supports C++17 |
| Qt | Qt 5 or Qt 6 |
| Qt Modules | Widgets, SQL, Charts |
| CMake | 3.16 or later |
| MySQL Server | MySQL 5.7+ or MySQL 8.x |
| Qt MySQL Driver | `QMYSQL` driver must be available |

You can check available SQL drivers from the application debug output. The program prints the available Qt SQL drivers at startup.

---

## Installation and Setup

### 1. Create the Database

Start MySQL and create the target database manually:

```sql
CREATE DATABASE IF NOT EXISTS social_media_system
DEFAULT CHARACTER SET utf8mb4
COLLATE utf8mb4_unicode_ci;
```

The application will automatically create the required tables after it successfully connects to the database.

### 2. Check Database Configuration

Open the following file:

```text
infrastructure/databasemanager.cpp
```

Check or update these values according to your local MySQL environment:

```cpp
constexpr const char* kDatabaseHost = "127.0.0.1";
constexpr int kDatabasePort = 3306;
constexpr const char* kDatabaseName = "social_media_system";
constexpr const char* kDatabaseUser = "root";
constexpr const char* kDatabasePassword = "123456";
```

### 3. Build with Qt Creator

1. Open Qt Creator.
2. Choose **Open Project**.
3. Select `CMakeLists.txt`.
4. Configure a Qt kit with Widgets, SQL, and Charts support.
5. Build the project.
6. Run the application.

### 4. Build with Command Line

From the project root directory:

```bash
cmake -S . -B build
cmake --build build
```

If Qt cannot be found automatically, specify the Qt CMake path:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="path/to/Qt/lib/cmake"
cmake --build build
```

The exact Qt path depends on your operating system and Qt installation directory.

### 5. Run the Application

Before running the application:

- Make sure MySQL Server is running.
- Make sure the `social_media_system` database exists.
- Make sure the Qt `QMYSQL` driver is available.
- Make sure the database username and password are correct.

When the application starts successfully, it will:

1. Connect to MySQL.
2. Create the `users`, `posts`, and `operation_logs` tables if they do not exist.
3. Add missing owner fields to old `posts` tables if needed.
4. Create the default administrator account if it does not already exist.
5. Display the login window.

---

## Usage Guide

### Login

1. Start the application.
2. Enter username and password.
3. Click **Login**.
4. After successful login, the main dashboard window will open.

Default administrator:

```text
Username: admin
Password: 123456
```

### User Management

Available to administrators only.

Administrators can:

- Create user accounts.
- Refresh the user list.
- Enable selected users.
- Disable selected users.
- Reset selected user passwords.

The default reset password is:

```text
123456
```

### Post Data Management

Users can manage post records from the **Post Data** page.

Supported operations:

- Add a post.
- Update a selected post.
- Delete a selected post.
- Search by content or account.
- Filter by platform.
- Import CSV data.
- Refresh the table.

Data permission rules:

- Administrators can view and manage all post records.
- Normal users can view and manage only their own records.

### Dashboard

The dashboard provides a visual overview of the accessible data:

- Total posts.
- Total interactions.
- Total views.
- Engagement rate.
- Platform post share.
- Platform interactions.
- Recent 14-day interaction trend.
- Top 5 popular posts.

### Analytics

The analytics page supports:

- Platform filtering.
- Start date and end date filtering.
- Summary statistics.
- Platform statistics.
- Date trends.
- Top posts by interactions.

### Export Reports

The export page supports:

- CSV report export.
- TXT report export.
- HTML report export.
- Report preview before saving.
- Platform and date range filters.
- Default export directory from settings.

### Operation Logs

Administrators can view system operation logs, including:

- Login records.
- User management operations.
- Post operations.
- CSV import records.
- Report export records.

Logs can be filtered by:

- Username.
- Action.
- Date range.

### Settings

The settings page supports:

- Selecting a default report export directory.
- Saving export settings locally.
- Viewing project description and version information.

---

## CSV Import Format

### Standard CSV Format

The standard CSV format uses one row for each post record.

```csv
platform,account,content,date,likes,comments,shares,views
Douyin,Official Account,New product released today,2026-06-13,120,35,18,5600
```

| Column | Required | Description |
|---|---|---|
| `platform` | Yes | Platform name, such as Douyin or Bilibili. |
| `account` | Yes | Account name or author name. |
| `content` | Yes | Post title, content, or summary. |
| `date` | Yes | Publish date. |
| `likes` | No | Number of likes. |
| `comments` | No | Number of comments. |
| `shares` | No | Number of shares. |
| `views` | No | Number of views, reads, or plays. |

Recommended date format:

```text
YYYY-MM-DD
```

### Supported Platform-Specific CSV Formats

The importer also recognizes selected platform export structures.

| Platform | Supported Data Type | Imported Fields |
|---|---|---|
| Douyin | Work list CSV | Work name, publish time, play count, likes, shares, comments |
| Bilibili | Trend CSV | Date, play count, likes, comments, shares |
| Xiaohongshu | Note list CSV | Note title, first publish time, views or exposure, likes, comments, shares |
| WeChat Official Account | Article detail CSV | Article title, reading count, likes, comments, shares, earliest trend date |

Fields that do not exist in the current `posts` table, such as favorites, followers gained, completion rate, average watch duration, or cover click rate, are not stored.

### CSV Import Notes

- The selected file must be a real CSV text file.
- Excel workbooks should be saved as CSV UTF-8 before import.
- Required fields must not be empty.
- Numeric values should be non-negative integers.
- Unsupported fields may be ignored.
- Imported records are automatically associated with the current logged-in user.

---

## Permission Rules

| Module | Administrator | Normal User |
|---|---:|---:|
| Login | Yes | Yes |
| Dashboard | Yes | Yes |
| Post Data Management | Yes | Yes |
| CSV Import | Yes | Yes |
| Data Analytics | Yes | Yes |
| Report Export | Yes | Yes |
| System Settings | Yes | Yes |
| User Management | Yes | No |
| Operation Logs | Yes | No |

Data visibility rules:

- Administrators can access all user data.
- Normal users can access only records created or imported by themselves.
- Old post records without owner information are visible to administrators only.

---

## Report Export

The system supports three report formats.

| Format | Suitable For | Description |
|---|---|---|
| CSV | Data processing | Can be opened by Excel, WPS, or other spreadsheet tools. |
| TXT | Simple report reading | Plain text report with summary and ranking sections. |
| HTML | Visual report display | Web-style report with styled sections and charts. |

Generated reports may include:

- Report generation time.
- Selected platform.
- Selected date range.
- Data scope.
- Summary statistics.
- Platform statistics.
- Top posts by interactions.

---

## Common Issues

### Database Connection Failed

Possible causes:

- MySQL Server is not running.
- The database `social_media_system` does not exist.
- The username or password in `databasemanager.cpp` is incorrect.
- The Qt `QMYSQL` driver is not installed or not found.

Suggested solutions:

1. Start MySQL Server.
2. Create the database manually.
3. Check the database host, port, username, and password.
4. Check whether the Qt MySQL driver is available.

### Login Failed

Possible causes:

- The username does not exist.
- The password is incorrect.
- The account has been disabled.

Suggested solutions:

- Use the default administrator account for first login.
- Ask an administrator to enable the account.
- Reset the user password if needed.

### CSV Import Failed

Possible causes:

- The file is not a real CSV text file.
- The file is an Excel workbook renamed as `.csv`.
- Required fields are missing.
- Date or numeric values are invalid.
- The platform-specific format is not supported.

Suggested solutions:

- Save the file as CSV UTF-8.
- Check whether the CSV header matches the supported format.
- Use valid dates and non-negative numeric values.
- Try importing a standard CSV file first.

### Build Failed

Possible causes:

- Qt is not configured correctly.
- Qt Charts or Qt SQL module is missing.
- CMake cannot find the Qt installation path.
- The compiler does not support C++17.

Suggested solutions:

- Use a Qt kit that includes Widgets, SQL, and Charts.
- Set `CMAKE_PREFIX_PATH` to the Qt CMake directory.
- Use a modern compiler with C++17 support.

---

## Future Improvements

- Add richer platform-specific fields such as favorites, followers gained, and completion rate.
- Add PDF export support.
- Add more chart types and customizable dashboard widgets.
- Add batch editing for post records.
- Add user password change functionality.
- Add advanced analytics such as growth trend, content category comparison, and platform performance prediction.
- Add external configuration file support for database settings.

---

## Team Members

| Name | Student ID | Responsibility |
|---|---|---|
| 吴裕勇 | 8002124023 | Team Leader |
| 熊倡 | 8002124024 | Member |
| 王旭坤 | 8002124022 | Member |
| 刘子懿 | 8002124019 | Member |

---

## License

This project is intended for coursework, learning, and educational demonstration purposes.

If the project is distributed publicly, please add an appropriate open-source license file before release.
