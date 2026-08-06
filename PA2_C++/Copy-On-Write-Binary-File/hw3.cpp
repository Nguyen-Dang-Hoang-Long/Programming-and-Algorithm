#ifndef __PROGTEST__
#include <cassert>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <iostream>
#endif /* __PROGTEST__ */

class CFile
{  
  private:
    // todo
    // Attributes
    struct CState {
      size_t size_ = 0, cap_ = 0, pos_ = 0, refcnt_ = 1;
      uint8_t * data_ = nullptr;

      CState () = default; // Con
      CState(size_t size, size_t cap, size_t pos) : // Con w para
        size_ (size), cap_ (cap), pos_ (pos), refcnt_ (1), data_(cap ? new uint8_t [cap] : nullptr) {}
      CState (const CState& a) = delete; // Copy disabled
      CState& operator = (const CState& a) = delete; // Op= disabled
      ~CState () {delete[] data_;} // Destructor
    };
    CState * state_;
    CState ** version_;
    size_t size_h, cap_h;

    // Helper functions
    void reset() { // Reset attributes to original state
      state_ = nullptr;
      version_ = nullptr;
      size_h = cap_h = 0;
    }

    void release () { // Reduce cnt of all by 1. If 0, delete the state and version versions.
      if (--(state_->refcnt_) == 0) delete state_; // Delete deep copy
      for (size_t i = 0; i < size_h; i++) if (--(version_[i]->refcnt_) == 0) delete version_[i]; // Delete deep copy
      delete [] version_; // Delete array of pointers (to the deep copy)
    }

    void copy (const CFile& file) { // Copy state, incre cnt, copy version, copy size & cap
      state_ = file.state_;
      state_->refcnt_++;

      version_ = file.cap_h ? new CState * [file.cap_h] : nullptr;
      for (size_t i = 0; i < file.size_h; i++) {
        version_[i] = file.version_[i];
        version_[i]->refcnt_++;
      }

      size_h = file.size_h;
      cap_h = file.cap_h;
    }

    void detach () { // Create a deep copy before modification
      if (state_->refcnt_ <= 1) return; // Check if its needed (0 = not needed, 1 = just modify current)
      CState * new_state = new CState (state_->size_, state_->cap_, state_->pos_); // New copy
      for (size_t i = 0; i < state_->size_; i++) new_state->data_[i] = state_->data_[i]; // Deep copy data
      if (--(state_->refcnt_) == 0) delete state_; // Reduce cnt to current shallow copy
      state_ = new_state; // Move to the modifiable deep copy
    }
  public:
                   CFile () : state_(new CState()), version_(nullptr), size_h(0), cap_h(0) {} // Con
                   // copy cons, dtor, op=
                   ~CFile () { // Destructor (only if refcnt == 0)
                    release(); 
                    reset();
                   }
                   CFile (const CFile& file) { // Shallow copy 
                    copy(file);
                   } 
                   CFile& operator = (const CFile& file) {// Shallow copy op= 
                    if (this == &file) return *this;
                    release();
                    reset();
                    copy(file);
                    return *this;
                   } 
                   
    bool           seek                       ( size_t           offset ) {
      if (offset > state_->size_) return false;
      detach();
      state_->pos_= offset;
      return true;
    }
    size_t         read                       ( uint8_t           dst[],
                                                size_t            bytes ) {
      // Read     
      if (bytes == 0) return bytes;
      size_t can_read = ((state_->size_ - state_->pos_) > bytes) ? (bytes) : (state_->size_ - state_->pos_); 
      for (size_t i = 0; i < can_read; i++) {
        dst[i] = state_->data_[state_->pos_ + i];
      } 

      // Advance pos
      state_->pos_ += can_read;

      // Return bytes read
      return can_read;
    }
    size_t         write                      ( const uint8_t     src[],
                                                size_t            bytes ) {
      // Create a deep copy
      if (bytes == 0) return bytes;
      detach();

      // Resize
      if (state_->pos_ + bytes >= state_->cap_) {
        state_->cap_ = (state_->pos_ + bytes)*2 + 1;
        uint8_t * new_data = new uint8_t [state_->cap_];
        for (size_t i = 0; i < state_->size_; i++) new_data[i] = state_->data_[i];
        delete [] state_->data_;
        state_->data_ = new_data; 
      }

      // Append new data
      for (size_t i = 0; i < bytes; i++) state_->data_[state_->pos_ + i] = src[i];

      // Update pos and size (if pos > size)
      state_->pos_ += bytes;
      if (state_->pos_ > state_->size_) state_->size_ = state_->pos_;

      // Return bytes written
      return bytes;
    }
    void           truncate                   () {
      detach();
      state_->size_ = state_->pos_;
    }
    size_t         fileSize                   () const {
      return state_->size_;
    }
    void           addVersion                 () {
      // Resize cap
      if (size_h >= cap_h) {
        cap_h = cap_h*2 + 1;
        CState ** new_arr = new CState * [cap_h];
        for (size_t i = 0; i < size_h; i++) new_arr[i] = version_[i];
        delete [] version_;
        version_ = new_arr;
      }

      // Shallow copy
      version_[size_h++] = state_;
      state_->refcnt_++;
    }
    bool           undoVersion                () {
      // No version to undo
      if (size_h == 0) return false;

      // Decrease cnt for current version, delete if no cnt left
      if (--(state_->refcnt_) == 0) delete state_;

      // Undo (we will lose the current/after versions)
      state_ = version_[--size_h];

      return true;
    }
};

#ifndef __PROGTEST__
bool               writeTest                  ( CFile           & x,
                                                const std::initializer_list<uint8_t> & data,
                                                size_t            wrLen )
{
  return x . write ( data . begin (), data . size () ) == wrLen;
}

bool               readTest                   ( CFile           & x,
                                                const std::initializer_list<uint8_t> & data,
                                                size_t            rdLen )
{
  uint8_t  tmp[100];
  uint32_t idx = 0;

  if ( x . read ( tmp, rdLen ) != data . size ())
    return false;
    
  for ( auto v : data )
    if ( tmp[idx++] != v )
      return false;
  return true;
}

int main ()
{
  CFile f0;

  assert ( writeTest ( f0, {10, 20, 30}, 3 ) );
  assert ( f0 . fileSize () == 3 );
  assert ( writeTest ( f0, {60, 70, 80}, 3 ) );
  assert ( f0 . fileSize () == 6 );
  assert ( f0 . seek ( 2 ));
  assert ( writeTest ( f0, {5, 4}, 2 ) );
  assert ( f0 . fileSize () == 6 );
  assert ( f0 . seek ( 1 ));
  assert ( readTest ( f0, {20, 5, 4, 70, 80}, 7 ));
  assert ( f0 . seek ( 3 ));
  f0 . addVersion();
  assert ( f0 . seek ( 6 ));
  assert ( writeTest ( f0, {100, 101, 102, 103}, 4 ) );
  f0 . addVersion();
  assert ( f0 . seek ( 5 ));
  CFile f1 ( f0 );
  f0 . truncate ();
  assert ( f0 . seek ( 0 ));
  assert ( readTest ( f0, {10, 20, 5, 4, 70}, 20 ));
  assert ( f0 . undoVersion () );
  assert ( f0 . seek ( 0 ));
  assert ( readTest ( f0, {10, 20, 5, 4, 70, 80, 100, 101, 102, 103}, 20 ));
  assert ( f0 . undoVersion () );
  assert ( f0 . seek ( 0 ));
  assert ( readTest ( f0, {10, 20, 5, 4, 70, 80}, 20 ));
  assert ( !f0 . seek ( 100 ));
  assert ( writeTest ( f1, {200, 210, 220}, 3 ) );
  assert ( f1 . seek ( 0 ));
  assert ( readTest ( f1, {10, 20, 5, 4, 70, 200, 210, 220, 102, 103}, 20 ));
  assert ( f1 . undoVersion () );
  assert ( f1 . undoVersion () );
  assert ( readTest ( f1, {4, 70, 80}, 20 ));
  assert ( !f1 . undoVersion () );
  return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */
