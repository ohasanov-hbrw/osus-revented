#include "cachebuilder/metadataParser.hpp"
#include "globals.hpp"
#include "time_util.hpp"
#include "utils.hpp"

#include <algorithm>
#include <dirent.h>
#include <filesystem>
#include <functional>
#include <iostream>
#include <stack>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <unordered_map>
#include <unordered_set>

#include "sqlite3.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#if defined(__3DS__) || defined(THREEDS_BUILD)
extern "C" {
int fileno(FILE *stream);
int ftruncate(int fd, off_t length);
}
#endif

struct StdioFile {
  sqlite3_file base;
  FILE *fp;
};

static int stdioClose(sqlite3_file *pFile) {
  StdioFile *f = (StdioFile *)pFile;
  if (f->fp) {
    fclose(f->fp);
    f->fp = nullptr;
  }
  return SQLITE_OK;
}

static int stdioRead(sqlite3_file *pFile, void *zBuf, int iAmt,
                     sqlite3_int64 iOfst) {
  StdioFile *f = (StdioFile *)pFile;
  fseek(f->fp, (long)iOfst, SEEK_SET);
  size_t readAmt = fread(zBuf, 1, iAmt, f->fp);
  if (readAmt == (size_t)iAmt)
    return SQLITE_OK;
  if (readAmt < (size_t)iAmt) {
    memset((char *)zBuf + readAmt, 0, iAmt - readAmt);
    return SQLITE_IOERR_SHORT_READ;
  }
  return SQLITE_IOERR_READ;
}

static int stdioWrite(sqlite3_file *pFile, const void *zBuf, int iAmt,
                      sqlite3_int64 iOfst) {
  StdioFile *f = (StdioFile *)pFile;
  fseek(f->fp, (long)iOfst, SEEK_SET);
  if (fwrite(zBuf, 1, iAmt, f->fp) == (size_t)iAmt)
    return SQLITE_OK;
  return SQLITE_IOERR_WRITE;
}

static int stdioTruncate(sqlite3_file *pFile, sqlite3_int64 size) {
  StdioFile *f = (StdioFile *)pFile;
  if (!f || !f->fp)
    return SQLITE_IOERR_TRUNCATE;

  fflush(f->fp);
  int fd = fileno(f->fp);
  if (fd < 0)
    return SQLITE_IOERR_TRUNCATE;

  return (ftruncate(fd, (off_t)size) == 0) ? SQLITE_OK : SQLITE_IOERR_TRUNCATE;
}

static int stdioSync(sqlite3_file *pFile, int flags) {
  StdioFile *f = (StdioFile *)pFile;
  fflush(f->fp);
  return SQLITE_OK;
}

static int stdioFileSize(sqlite3_file *pFile, sqlite3_int64 *pSize) {
  StdioFile *f = (StdioFile *)pFile;
  fseek(f->fp, 0, SEEK_END);
  *pSize = ftell(f->fp);
  return SQLITE_OK;
}

static int stdioLock(sqlite3_file *pFile, int eLock) { return SQLITE_OK; }
static int stdioUnlock(sqlite3_file *pFile, int eLock) { return SQLITE_OK; }
static int stdioCheckReservedLock(sqlite3_file *pFile, int *pResOut) {
  *pResOut = 0;
  return SQLITE_OK;
}
static int stdioFileControl(sqlite3_file *pFile, int op, void *pArg) {
  return SQLITE_NOTFOUND;
}
static int stdioSectorSize(sqlite3_file *pFile) { return 512; }
static int stdioDeviceCharacteristics(sqlite3_file *pFile) { return 0; }

static const sqlite3_io_methods stdioIoMethods = {1,
                                                  stdioClose,
                                                  stdioRead,
                                                  stdioWrite,
                                                  stdioTruncate,
                                                  stdioSync,
                                                  stdioFileSize,
                                                  stdioLock,
                                                  stdioUnlock,
                                                  stdioCheckReservedLock,
                                                  stdioFileControl,
                                                  stdioSectorSize,
                                                  stdioDeviceCharacteristics};

static int stdioOpen(sqlite3_vfs *pVfs, const char *zName, sqlite3_file *pFile,
                     int flags, int *pOutFlags) {
  StdioFile *f = (StdioFile *)pFile;
  memset(f, 0, sizeof(StdioFile));
  if (!zName)
    return SQLITE_CANTOPEN;

  FILE *fp = fopen(zName, "r+b");
  if (!fp && (flags & SQLITE_OPEN_CREATE)) {
    fp = fopen(zName, "w+b");
    if (fp) {
      fclose(fp);
      fp = fopen(zName, "r+b");
    }
  }

  if (!fp)
    return SQLITE_CANTOPEN;

  f->base.pMethods = &stdioIoMethods;
  f->fp = fp;
  if (pOutFlags)
    *pOutFlags = flags;
  return SQLITE_OK;
}

static int stdioDelete(sqlite3_vfs *pVfs, const char *zName, int syncDir) {
  remove(zName);
  return SQLITE_OK;
}

static int stdioAccess(sqlite3_vfs *pVfs, const char *zName, int flags,
                       int *pResOut) {
  FILE *fp = fopen(zName, "rb");
  if (fp) {
    fclose(fp);
    *pResOut = 1;
  } else {
    *pResOut = 0;
  }
  return SQLITE_OK;
}

static int stdioFullPathname(sqlite3_vfs *pVfs, const char *zIn, int nOut,
                             char *zOut) {
  // Pass sdmc:/ paths straight through to fopen without POSIX resolution
  snprintf(zOut, nOut, "%s", zIn);
  return SQLITE_OK;
}

void register3DSVfs() {
  static sqlite3_vfs stdioVfs;
  memset(&stdioVfs, 0, sizeof(sqlite3_vfs));
  stdioVfs.iVersion = 1;
  stdioVfs.szOsFile = sizeof(StdioFile);
  stdioVfs.mxPathname = 512;
  stdioVfs.zName = "3ds-stdio";
  stdioVfs.xOpen = stdioOpen;
  stdioVfs.xDelete = stdioDelete;
  stdioVfs.xAccess = stdioAccess;
  stdioVfs.xFullPathname = stdioFullPathname;

  sqlite3_vfs_register(&stdioVfs, 1); // Set as default VFS
}

// Global storage
std::unordered_map<int, std::vector<FileMetadata>> namesOfSets;
std::unordered_map<int, int> numberOfMaps;
std::atomic<int> numBeatmapsFound;
static size_t mapsInCurrentBatch = 0;

// Helper functions
bool dirExists(const std::string &path) {
  struct stat st;
  return stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

void createDir(const std::string &path) {
#if defined(THREEDS_BUILD) || defined(__linux__)
  mkdir(path.c_str(), 0755);
#else
  mkdir(path.c_str());
#endif
}

bool fileExists(const std::string &path) {
  struct stat st;
  return stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

void normalizePath(std::string &path) {
  for (char &c : path) {
    if (c == '\\')
      c = '/';
  }
}

struct DirState {
  std::string path;
  std::vector<std::string> entries;
  size_t index;
};

#ifdef THREEDS_BUILD
constexpr size_t BATCH_SET_LIMIT = 5;
constexpr size_t BATCH_MAP_LIMIT = 50;
#else
constexpr size_t BATCH_SET_LIMIT = 20;
constexpr size_t BATCH_MAP_LIMIT = 200;
#endif

static void ensureParentDirExists(const std::string &filePath) {
  std::string path = filePath;
  normalizePath(path);

  size_t lastSlash = path.find_last_of('/');
  if (lastSlash == std::string::npos)
    return;

  std::string dir = path.substr(0, lastSlash);
  if (dir.empty() || dir == "sdmc:" || dir == "sdmc:/" || dir == "/")
    return;

  // Recursively create parent directories
  size_t pos = 0;
  while ((pos = dir.find('/', pos)) != std::string::npos) {
    if (pos > 0) {
      std::string sub = dir.substr(0, pos);
      if (!sub.empty() && sub != "sdmc:" && sub != "sdmc:/" && sub != "/" &&
          !dirExists(sub)) {
        createDir(sub);
      }
    }
    pos++;
  }
  if (!dirExists(dir)) {
    createDir(dir);
  }
}

static sqlite3 *openSetDatabase(const std::string &dbPath) {
  std::string fullPath = dbPath;
  normalizePath(fullPath);

  // If path doesn't end in .db, append /beatmapsets.db
  if (fullPath.size() < 3 ||
      fullPath.compare(fullPath.size() - 3, 3, ".db") != 0) {
    fullPath += "/beatmapsets.db";
    normalizePath(fullPath);
  }

#if defined(__3DS__) || defined(THREEDS_BUILD)
  // Ensure explicit sdmc:/ prefix for devkitARM FatFs
  if (fullPath.rfind("sdmc:", 0) != 0) {
    if (!fullPath.empty() && fullPath[0] == '/') {
      fullPath = "sdmc:" + fullPath;
    } else {
      fullPath = "sdmc:/" + fullPath;
    }
  }
#endif

  // Ensure database directory exists before SQLite tries to open the file
  ensureParentDirExists(fullPath);

  sqlite3 *db = nullptr;
  int rc = SQLITE_OK;

#if defined(__3DS__) || defined(THREEDS_BUILD)

  register3DSVfs();

  // Pass raw sdmc:/ path directly with unix-none VFS to bypass URI mangling and
  // file locks
  rc = sqlite3_open_v2(fullPath.c_str(), &db,
                       SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr);
#else
  // Windows / PC URI path with nolock support
  std::string uriPath = "file:";
  if (fullPath.rfind("//", 0) == 0) {
    uriPath += "//" + fullPath + "?nolock=1";
  } else {
    uriPath += "///" + fullPath + "?nolock=1";
  }

  rc = sqlite3_open_v2(
      uriPath.c_str(), &db,
      SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, nullptr);
#endif

  if (rc != SQLITE_OK) {
    std::cerr << "[DATABASE ERROR] Failed to open database (" << fullPath
              << "): " << (db ? sqlite3_errmsg(db) : "Unknown error")
              << std::endl;
    if (db)
      sqlite3_close(db);
    return nullptr;
  }

  // In-memory journaling and relaxed sync for SD card performance
  sqlite3_busy_timeout(db, 3000);
  sqlite3_exec(db, "PRAGMA journal_mode = MEMORY;", nullptr, nullptr, nullptr);
  sqlite3_exec(db, "PRAGMA synchronous = OFF;", nullptr, nullptr, nullptr);
  sqlite3_exec(db, "PRAGMA cache_size = -500;", nullptr, nullptr, nullptr);

  const char *schema = R"(
    CREATE TABLE IF NOT EXISTS beatmapsets (
      setid INTEGER PRIMARY KEY,
      title TEXT,
      maps INTEGER,
      ids TEXT,
      artists TEXT,
      creators TEXT
    );
  )";

  char *errMsg = nullptr;
  if (sqlite3_exec(db, schema, nullptr, nullptr, &errMsg) != SQLITE_OK) {
    std::cerr << "[DATABASE ERROR] Schema creation failed: "
              << (errMsg ? errMsg : "unknown") << std::endl;
    sqlite3_free(errMsg);
  }

  return db;
}

static void upsertSetInDb(sqlite3 *db, int setid, const std::string &title,
                          int numMaps,
                          const std::vector<FileMetadata> &metadataList) {
  if (!db)
    return;

  std::string idsList, artistsList, creatorsList;
  std::unordered_set<std::string> uniqueArtists, uniqueCreators;

  for (size_t i = 0; i < metadataList.size(); ++i) {
    if (i > 0)
      idsList += ",";
    idsList += std::to_string(metadataList[i].id);

    if (uniqueArtists.insert(metadataList[i].artist).second) {
      if (!artistsList.empty())
        artistsList += ", ";
      artistsList += metadataList[i].artist;
    }
    if (uniqueCreators.insert(metadataList[i].creator).second) {
      if (!creatorsList.empty())
        creatorsList += ", ";
      creatorsList += metadataList[i].creator;
    }
  }

  const char *sql = R"(
    INSERT OR REPLACE INTO beatmapsets (setid, title, maps, ids, artists, creators)
    VALUES (?, ?, ?, ?, ?, ?);
  )";

  sqlite3_stmt *stmt = nullptr;
  int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
  if (rc != SQLITE_OK) {
    std::cerr << "[DATABASE ERROR] Prepare failed: " << sqlite3_errmsg(db)
              << std::endl;
    return;
  }

  sqlite3_bind_int(stmt, 1, setid);
  sqlite3_bind_text(stmt, 2, title.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_int(stmt, 3, numMaps);
  sqlite3_bind_text(stmt, 4, idsList.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 5, artistsList.c_str(), -1, SQLITE_TRANSIENT);
  sqlite3_bind_text(stmt, 6, creatorsList.c_str(), -1, SQLITE_TRANSIENT);

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    std::cerr << "[DATABASE ERROR] Step failed: " << sqlite3_errmsg(db)
              << std::endl;
  }

  sqlite3_finalize(stmt);
}

void flushBatch() {
  std::cout << "\e[1;35m[DATABASE] \e[38;5;236mFlushing current batch"
            << std::endl;
  if (namesOfSets.empty())
    return;

  if (!dirExists(Global.DatabaseLocation)) {
    createDir(Global.DatabaseLocation);
  }

  processAllSetImages();

  sqlite3 *db = openSetDatabase(Global.DatabaseLocation);
  if (db) {
    sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
  }

  while (!namesOfSets.empty()) {
    auto node = namesOfSets.extract(namesOfSets.begin());
    int setid = node.key();
    std::vector<FileMetadata> &newMaps = node.mapped();

    std::string setDir = Global.DatabaseLocation + "/" + std::to_string(setid);
    normalizePath(setDir);

    std::cout << "\e[1;35m[DATABASE] \e[38;5;236mWriting " << setid
              << std::endl;

    std::vector<FileMetadata> combinedList = std::move(newMaps);

    if (dirExists(setDir)) {
      std::vector<FileMetadata> existingMaps =
          parseCachedMaps(Global.DatabaseLocation, setid);
      for (auto &existing : existingMaps) {
        bool exists = false;
        for (const auto &m : combinedList) {
          if (m.id == existing.id) {
            exists = true;
            break;
          }
        }
        if (!exists) {
          combinedList.push_back(std::move(existing));
        }
      }
    }

    std::unordered_map<std::string, int> selection;
    for (const auto &file : combinedList) {
      selection[file.title]++;
    }
    std::string title = "error";
    int maxValue = -1;
    for (const auto &[key, value] : selection) {
      if (value > maxValue) {
        maxValue = value;
        title = key;
      }
    }

    // Write individual map .db text files
    writeBeatmapFile(setid, combinedList);

    // Update SQLite set index
    if (db) {
      upsertSetInDb(db, setid, title, static_cast<int>(combinedList.size()),
                    combinedList);
    }
  }

  if (db) {
    sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    sqlite3_close(db);
  }

  namesOfSets.clear();
  mapsInCurrentBatch = 0;
  std::cout << "\e[1;35m[DATABASE] \e[38;5;236mFlushed" << std::endl;
}

void decideNamesForSets() { flushBatch(); }

int generate9DigitHash(const std::string &input) {
  size_t rawHash = std::hash<std::string>{}(input);
  return 100000000 + static_cast<int>(rawHash % 900000000);
}

void addFileToMap(const std::string &path) {
  std::vector<std::string> output = ParseNameFile(path);
  if (output.size() < 6)
    return;

  int parsedSetId =
      static_cast<int>(std::strtol(output[4].c_str(), nullptr, 10));
  int parsedId = static_cast<int>(std::strtol(output[5].c_str(), nullptr, 10));

  if (parsedSetId <= 0) {
    std::filesystem::path p(path);
    std::string folderName = p.parent_path().filename().string();
    parsedSetId = generate9DigitHash(folderName);
  }

  if (parsedId <= 0) {
    std::filesystem::path p(path);
    std::string fileName = p.filename().string();
    parsedId = generate9DigitHash(fileName);
  }

  FileMetadata temp{.path = path,
                    .title = std::move(output[0]),
                    .artist = std::move(output[1]),
                    .creator = std::move(output[2]),
                    .version = std::move(output[3]),
                    .setid = parsedSetId,
                    .id = parsedId,
                    .bgImage = extractBackgroundImage(path),
                    .coverFile = " "};

  namesOfSets[temp.setid].push_back(std::move(temp));
  mapsInCurrentBatch++;
}

void buildFileMap(const std::string &rootPath) {
  std::string root = rootPath;
  normalizePath(root);
  if (!root.empty() && root.back() == '/')
    root.pop_back();

  std::cout << "\e[1;35m[DATABASE] \e[38;5;236mStarting search at " << root
            << std::endl;
  numBeatmapsFound = 0;

  auto readEntries =
      [](const std::string &dirPath) -> std::vector<std::string> {
    std::vector<std::string> result;
    DIR *dir = opendir(dirPath.c_str());
    if (!dir)
      return result;

    struct dirent *de;
    while ((de = readdir(dir)) != nullptr) {
      if (de->d_name[0] != '.')
        result.emplace_back(de->d_name);
    }
    closedir(dir);
    return result;
  };

  std::stack<DirState> stack;
  stack.push({root, readEntries(root), 0});

  while (!stack.empty()) {
    DirState &state = stack.top();

    if (state.index < state.entries.size()) {
      std::string entryName = std::move(state.entries[state.index]);
      state.index++;

      std::string fullPath = state.path + "/" + entryName;
      normalizePath(fullPath);

      struct stat st;
      if (stat(fullPath.c_str(), &st) == 0) {
        if (S_ISDIR(st.st_mode)) {
          DirState newState;
          newState.path = fullPath;
          newState.entries = readEntries(fullPath);
          newState.index = 0;
          stack.push(std::move(newState));
          continue;
        } else if (S_ISREG(st.st_mode) &&
                   IsFileExtension(entryName.c_str(), ".osu")) {
          numBeatmapsFound = numBeatmapsFound + 1;
          addFileToMap(fullPath);

          if (mapsInCurrentBatch >= BATCH_MAP_LIMIT ||
              namesOfSets.size() >= BATCH_SET_LIMIT) {
            flushBatch();
          }
        }
      }
    } else {
      stack.pop();
    }
  }

  flushBatch();
}

void writeBeatmapFile(int setid,
                      const std::vector<FileMetadata> &metadataList) {
  std::string dirName = Global.DatabaseLocation + "/" + std::to_string(setid);
  normalizePath(dirName);
  if (!dirExists(dirName))
    createDir(dirName);

  for (const auto &fileInfo : metadataList) {
    std::string filePath = dirName + "/" + std::to_string(fileInfo.id) + ".db";
    FILE *file = fopen(filePath.c_str(), "w");
    if (!file)
      continue;

    fprintf(file,
            "Path:%s\nTitle:%s\nArtist:%s\nCreator:%s\nVersion:%s\nBeatmapID:%"
            "d\nBeatmapSetID:%d\nCoverFile:%s\n",
            fileInfo.path.c_str(), fileInfo.title.c_str(),
            fileInfo.artist.c_str(), fileInfo.creator.c_str(),
            fileInfo.version.c_str(), fileInfo.id, setid,
            fileInfo.coverFile.c_str());
    fclose(file);
  }
}

void writeBeatmapSetFile(const std::string &filename, int beatmapSetId,
                         const std::string &title, int numMaps,
                         const std::vector<FileMetadata> &metadataListObj) {
  sqlite3 *db = openSetDatabase(filename);
  if (!db)
    return;

  upsertSetInDb(db, beatmapSetId, title, numMaps, metadataListObj);
  sqlite3_close(db);
}

void clearFileMap() {
  namesOfSets.clear();
  numberOfMaps.clear();
}

void listAllMaps() {}

std::vector<SetFileMetadata> parseCachedSets(const std::string &dbPath) {
  std::vector<SetFileMetadata> metadataList;
  sqlite3 *db = openSetDatabase(dbPath);
  if (!db)
    return metadataList;

  const char *sql =
      "SELECT setid, title, maps, artists, creators FROM beatmapsets;";
  sqlite3_stmt *stmt = nullptr;

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      SetFileMetadata s;
      s.setid = sqlite3_column_int(stmt, 0);
      const char *t =
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
      s.title = t ? t : "";
      s.number = sqlite3_column_int(stmt, 2);
      const char *a =
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
      s.artists = a ? a : "";
      const char *c =
          reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
      s.creators = c ? c : "";
      metadataList.push_back(std::move(s));
    }
  }

  if (stmt) {
    sqlite3_finalize(stmt);
  }
  sqlite3_close(db);
  return metadataList;
}

std::vector<FileMetadata> parseCachedMaps(const std::string &dbPath,
                                          int setid) {
  std::vector<FileMetadata> result;
  std::string setDir = dbPath + "/" + std::to_string(setid);
  normalizePath(setDir);

  DIR *dir = opendir(setDir.c_str());
  if (!dir)
    return result;

  struct dirent *entry;
  while ((entry = readdir(dir)) != nullptr) {
    std::string name = entry->d_name;
    if (name[0] == '.' || name.size() < 3 ||
        name.compare(name.size() - 3, 3, ".db") != 0)
      continue;

    std::string filePath = setDir + "/" + name;
    normalizePath(filePath);

    struct stat st;
    if (stat(filePath.c_str(), &st) != 0 || !S_ISREG(st.st_mode))
      continue;

    FILE *file = fopen(filePath.c_str(), "r");
    if (!file)
      continue;

    FileMetadata meta{.setid = setid, .id = 0};
    char line[1024];

    while (fgets(line, sizeof(line), file)) {
      std::string lineStr(line);
      lineStr.erase(lineStr.find_last_not_of("\r\n") + 1);
      size_t colon = lineStr.find(':');
      if (colon == std::string::npos)
        continue;

      std::string key = lineStr.substr(0, colon);
      std::string value = lineStr.substr(colon + 1);

      if (key == "Path")
        meta.path = value;
      else if (key == "Title")
        meta.title = value;
      else if (key == "Artist")
        meta.artist = value;
      else if (key == "Creator")
        meta.creator = value;
      else if (key == "Version")
        meta.version = value;
      else if (key == "BeatmapID")
        meta.id = std::stoi(value);
      else if (key == "CoverFile")
        meta.coverFile = value;
    }
    fclose(file);

    if (meta.id != 0)
      result.push_back(std::move(meta));
  }
  closedir(dir);

  std::sort(
      result.begin(), result.end(),
      [](const FileMetadata &a, const FileMetadata &b) { return a.id < b.id; });
  return result;
}

std::string extractBackgroundImage(const std::string &osuPath) {
  FILE *file = fopen(osuPath.c_str(), "r");
  if (!file)
    return "";

  char line[1024];
  bool inEvents = false;
  std::string bgFile;

  while (fgets(line, sizeof(line), file)) {
    std::string lineStr(line);

    size_t last = lineStr.find_last_not_of("\r\n\t ");
    if (last != std::string::npos)
      lineStr.erase(last + 1);
    else
      lineStr.clear();

    if (lineStr == "[Events]") {
      inEvents = true;
      continue;
    }

    if (inEvents && lineStr.rfind("//", 0) == 0)
      continue;

    if (inEvents && !lineStr.empty()) {
      if (lineStr.front() == '[' && lineStr != "[Events]")
        break;

      size_t first = lineStr.find('"');
      if (first != std::string::npos) {
        size_t second = lineStr.find('"', first + 1);
        if (second != std::string::npos) {
          std::string candidate = lineStr.substr(first + 1, second - first - 1);

          size_t start = candidate.find_first_not_of(" \t");
          if (start != std::string::npos) {
            candidate = candidate.substr(start);
          } else {
            candidate.clear();
          }

          if (candidate.empty())
            continue;

          std::string lower = candidate;
          for (char &c : lower)
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));

          bool isVideo = false;
          static const std::string videoExts[] = {
              ".mp4", ".avi", ".flv", ".mkv", ".mov", ".wmv", ".m4v"};
          for (const auto &ext : videoExts) {
            if (lower.size() >= ext.size() &&
                lower.compare(lower.size() - ext.size(), ext.size(), ext) ==
                    0) {
              isVideo = true;
              break;
            }
          }

          if (!isVideo) {
            bgFile = candidate;
            break;
          }
        }
      }
    }
  }

  fclose(file);
  return bgFile;
}

void processAllSetImages() {
  std::cout << "\e[1;35m[DATABASE] \e[38;5;236mProcessing images in sets "
            << namesOfSets.size() << std::endl;
  for (auto &[setid, metadataList] : namesOfSets) {
    std::unordered_map<std::string, std::string> bgToCover;
    int coverIndex = 0;

    for (auto &file : metadataList) {
      if (file.bgImage.empty()) {
        continue;
      }

      std::string beatmapDir = file.path;
      size_t lastSlash = beatmapDir.find_last_of("/\\");
      if (lastSlash != std::string::npos)
        beatmapDir = beatmapDir.substr(0, lastSlash);

      normalizePath(beatmapDir);

      std::string fullBgPath = beatmapDir + "/" + file.bgImage;
      normalizePath(fullBgPath);

      if (!fileExists(fullBgPath)) {
        std::string upperBg = file.bgImage;
        for (char &c : upperBg)
          c = std::toupper((unsigned char)c);

        std::string testPath = beatmapDir + "/" + upperBg;
        normalizePath(testPath);

        if (fileExists(testPath)) {
          fullBgPath = testPath;
        } else {
          size_t dotPos = file.bgImage.find_last_of('.');
          if (dotPos != std::string::npos) {
            std::string extUpperBg = file.bgImage;
            for (size_t i = dotPos; i < extUpperBg.length(); ++i) {
              extUpperBg[i] = std::toupper((unsigned char)extUpperBg[i]);
            }
            testPath = beatmapDir + "/" + extUpperBg;
            normalizePath(testPath);

            if (fileExists(testPath)) {
              fullBgPath = testPath;
            }
          }
        }

        if (!fileExists(fullBgPath)) {
          continue;
        }
      }

      auto it = bgToCover.find(fullBgPath);
      if (it != bgToCover.end()) {
        file.coverFile = it->second;
        continue;
      }

      std::string coverName = "cover_" + std::to_string(setid) + "_" +
                              std::to_string(coverIndex++) + ".bmp";
      std::string setDir =
          Global.DatabaseLocation + "/" + std::to_string(setid);
      normalizePath(setDir);

      std::string outPath = setDir + "/" + coverName;
      normalizePath(outPath);

      bool alreadyValid = false;

      if (FileExists(outPath.c_str())) {
        Image existing = LoadImage(outPath.c_str());
        if (existing.data != nullptr) {
          if (existing.width == COVER_WIDTH &&
              existing.height == COVER_HEIGHT) {
            alreadyValid = true;
          }
          UnloadImage(&existing);
        }
      }

      if (!alreadyValid) {
        Image img = LoadImage(fullBgPath.c_str());
        if (img.data == nullptr) {
          continue;
        }
        float targetAspect = (float)COVER_WIDTH / COVER_HEIGHT;
        float imageAspect = (float)img.width / img.height;
        Rectangle cropRect;
        if (imageAspect > targetAspect) {
          int cropWidth = (int)(img.height * targetAspect);
          cropRect = {(float)(img.width - cropWidth) / 2.0f, 0.0f,
                      (float)cropWidth, (float)img.height};
        } else {
          int cropHeight = (int)(img.width / targetAspect);
          cropRect = {0.0f, (float)(img.height - cropHeight) / 2.0f,
                      (float)img.width, (float)cropHeight};
        }
        ImageCrop(&img, cropRect);
        ImageResize(&img, COVER_WIDTH, COVER_HEIGHT);

        if (!dirExists(setDir))
          createDir(setDir);

        ExportImage(img, outPath.c_str());
        UnloadImage(&img);
      }
      else{
        std::cout << "\e[1;35m[DATABASE] \e[38;5;236mSkipping  "
            << fullBgPath << std::endl;
      }


      bgToCover[fullBgPath] = coverName;
      file.coverFile = coverName;
    }
  }
}

bool appendSingleBeatmap(const std::string &osuPath) {
  std::vector<std::string> output = ParseNameFile(osuPath);
  if (output.size() < 6)
    return false;

  FileMetadata meta{
      .path = osuPath,
      .title = std::move(output[0]),
      .artist = std::move(output[1]),
      .creator = std::move(output[2]),
      .version = std::move(output[3]),
      .setid = static_cast<int>(std::strtol(output[4].c_str(), nullptr, 10)),
      .id = static_cast<int>(std::strtol(output[5].c_str(), nullptr, 10)),
      .bgImage = extractBackgroundImage(osuPath),
      .coverFile = " "};

  writeBeatmapFile(meta.setid, {meta});

  std::vector<FileMetadata> existingMaps =
      parseCachedMaps(Global.DatabaseLocation, meta.setid);

  bool found = false;
  for (const auto &m : existingMaps) {
    if (m.id == meta.id) {
      found = true;
      break;
    }
  }
  if (!found)
    existingMaps.push_back(meta);

  sqlite3 *db = openSetDatabase(Global.DatabaseLocation);
  if (!db)
    return false;

  upsertSetInDb(db, meta.setid, meta.title,
                static_cast<int>(existingMaps.size()), existingMaps);
  sqlite3_close(db);

  return true;
}