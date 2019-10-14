#include "../src/mesh_obj.h"

#include <cassert>
#include <climits>
#include <cstdio>
#include <iostream>

/*!re2c
  re2c:indent:string = '  ';
  re2c:define:YYCTYPE = char;
  re2c:yyfill:enable = 0;
  re2c:flags:tags = 1;

  D = [0-9];
  N = D+;                           // natural number
  R = ('-'? (D+'.'?D* | D*'.'?D+)); // real number
  S = [ \t];                        // whitespace
  E = ([\n\r]+ | '\x00');           // end of line or null terminator
*/

#define YYMTAGP(tags) tags.push_back(YYCURSOR);

namespace {
  typedef std::vector<const char*> Mtag;

  // MtagPtr wraps an Mtag vector and overrides operator= to take a pointer
  // rather than copying the value. Then we can have an MtagPtr called "tags",
  // and when re2c generates the code "tags = yyt<N>", it will invoke our
  // operator=, and avoid making a copy of the yyt<N>.
  class MtagPtr {
  public:
    MtagPtr() : ptr_(nullptr) {}

    Mtag& operator=(Mtag& o) {
      ptr_ = &o;
      return o;
    }

    Mtag* operator->() {
      return ptr_;
    }

    const char* operator[](int i) {
      return (*ptr_)[i];
    }

    Mtag::iterator begin() { return ptr_->begin(); }
    Mtag::iterator end() { return ptr_->end(); }

  private:
    Mtag* ptr_;
  };
}

void WavFrObj::ParseFrom(const std::string& input) {
  Clear();

  const char* const start = input.c_str();

  const char *YYMARKER = nullptr;
  const char *YYCURSOR = start;
  const char *YYLIMIT = YYCURSOR + input.size();

  // tag variables
  const char *name_start, *name_end; // demarcate an object name
  const char /*!stags:re2c format = "*@@ = nullptr"; separator = ", ";*/;
  Mtag /*!mtags:re2c format = "@@"; separator = ", ";*/;

  // After matching, re2c will set "tags" to refer to one of the m-tag vectors,
  // above.
  MtagPtr tags;

  static const char* const badFaceMsg =
    "WavFrObj::ParseFrom failed: face with out-of-bounds value at offset ";

  // lambda which gets the current offset into the input
  auto getOffset = [&]() -> uintptr_t { return uintptr_t(YYCURSOR - start); };

  int total_faces = 0;

  while(YYCURSOR < YYLIMIT) {
    /*!re2c
    * { 
      std::cout << "WavFrObj::ParseFrom failed: unrecognized line at offset "
        << getOffset() << std::endl;
      Clear();
      return;
    }

    'v' (S+ #tags R){3} E {
      assert(tags->size() == 3);
      verts_.push_back(Vector3f{
        strtof(tags[0], nullptr),
        strtof(tags[1], nullptr),
        strtof(tags[2], nullptr)
      });
      tags->clear();
      continue;
    }

    'vt' (S+ #tags R){2} E {
      assert(tags->size() == 2);
      float u = strtof(tags[0], nullptr);
      float v = strtof(tags[1], nullptr);
      uvs_.emplace_back(u, 1.0f - v);
      tags->clear();
      continue;
    }

    'vn' (S+ #tags R){3} E {
      assert(tags->size() == 3);
      normals_.emplace_back(Vector3f{
        strtof(tags[0], nullptr),
        strtof(tags[1], nullptr),
        strtof(tags[2], nullptr)
      });
      tags->clear();
      continue;
    }

    // Parse a line of the form "f v v v...". Each element of the "tags"
    // vector points to the start of a vertex number "v".
    'f' (S+ #tags N){3,} E {
      assert(tags->size() >= 3);
      std::vector<ObjVert> face_verts;
      face_verts.reserve(tags->size());
      for(const char* str: tags) {
        unsigned long v = strtoul(str, nullptr, 10);
        if(v == 0 || v > INT_MAX) {
          std::cout << badFaceMsg << getOffset() << std::endl;
          return;
        }
        face_verts.push_back({int(v)-1, -1, -1});
      }
      tags->clear();
      AddFaceToCurrentObject(std::move(face_verts));
      total_faces++;
      continue;
    }

    // Parse a line of the form "f v/t v/t v/t...". Each element of "tags"
    // points to the start of a vertex/texture coordinate pair "v/t".
    'f' (S+ #tags N '/' N){3,} E {
      assert(tags->size() >= 3);
      std::vector<ObjVert> face_verts;
      face_verts.reserve(tags->size());
      for(const char* str: tags) {
        unsigned long v, t;
        assert(2 == sscanf(str, "%lu/%lu", &v, &t)); // Parse a v/t pair.
        if(v == 0 || v > INT_MAX ||
           t == 0 || t > INT_MAX) {
          std::cout << badFaceMsg << getOffset() << std::endl;
          return;
        }
        face_verts.push_back({int(v)-1, int(t)-1, -1});
      }
      tags->clear();
      AddFaceToCurrentObject(std::move(face_verts));
      total_faces++;
      continue;
    }

    // Parse a line of the form "f v//n v//n v//n...". Each element of the
    // "tags" vector points to the start of a vertex/normal pair "v//n".
    'f' (S+ #tags N '//' N){3,} E {
      assert(tags->size() >= 3);
      std::vector<ObjVert> face_verts;
      face_verts.reserve(tags->size());
      for(const char* str: tags) {
        unsigned long v, n;
        assert(2 == sscanf(str, "%zu//%zu", &v, &n)); // Parse a v//n pair.
        if(v == 0 || v > INT_MAX ||
           n == 0 || n > INT_MAX) {
          std::cout << badFaceMsg << getOffset() << std::endl;
          return;
        }
        face_verts.push_back({int(v)-1, -1, int(n)-1});
      }
      tags->clear();
      AddFaceToCurrentObject(std::move(face_verts));
      total_faces++;
      continue;
    }

    // Parse a line of the form "f v/t/n v/t/n v/t/n...". Each element of the
    // "tags" vector points to the start of a "v/t/n" triple.
    'f' (S+ #tags N '/' N '/' N){3,} E {
      assert(tags->size() >= 3);
      std::vector<ObjVert> face_verts;
      face_verts.reserve(tags->size());
      for(const char* str: tags) {
        unsigned long v, t, n;
        // Parse a v/t/n triple.
        assert(3 == sscanf(str, "%zu/%zu/%zu", &v, &t, &n));
        if(v == 0 || v > INT_MAX ||
           t == 0 || t > INT_MAX ||
           n == 0 || n > INT_MAX) {
          std::cout << badFaceMsg << getOffset() << std::endl;
          return;
        }
        face_verts.push_back({int(v)-1, int(t)-1, int(n)-1});
      }
      tags->clear();
      AddFaceToCurrentObject(std::move(face_verts));
      total_faces++;
      continue;
    }

    'o' S+ @name_start .* @name_end E {
      assert(name_start < name_end);
      std::string name = std::string(name_start, name_end - name_start);
      std::cout << "WavFrObj::ParseFrom found object \"" << name << "\"\n";
      objects_.emplace_back(std::move(name));
      continue;
    }

    // ignore comments, groups, smoothing, & materials
    [#gs(mtllib)(usemtl)] .* E {
      continue;
    }

    E {
      continue;
    }
    */
  }

  std::cout << "WavFrObj::ParseFrome found " << objects_.size()
    << " objects, " << verts_.size() << " vertices, " << uvs_.size()
    << " UVs, " << normals_.size() << " normals, " << total_faces << " faces\n";

  Sanitize();
}
