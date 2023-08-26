#ifndef BYTESTREAMMACROS_H
#define BYTESTREAMMACROS_H

// Macros for ByteStream

#define declBYTESTREAM                                          \
    ByteStream      ToByteStream() const;                       \
    void            FromByteStream( ByteStream & inStream );

#ifdef _WIN32
#define startBTO(C) ByteStream C ## ::ToByteStream() const { ByteStream BStream;
#define endBTO return std::move(BStream); }
#define startBFROM(C) void C ## ::FromByteStream( ByteStream & inStream ) {
#define endBFROM }
#else
#define startBTO(C) ByteStream C::ToByteStream() const { ByteStream BStream;
#define endBTO return std::move(BStream); }
#define startBFROM(C) void C::FromByteStream( ByteStream & inStream ) {
#define endBFROM }
#endif

#define ABR(T,A) BStream.Add ## T( #A, A );
#define GBR(T,A) inStream.Get ## T( #A, A );

#ifdef _WIN32
#define ABRS(A) BStream.AddStream( #A, A ## .ToByteStream() );
#define GBRS(A) { ByteStream tmpS(true); if ( inStream.GetStream( #A, tmpS ) ) { A ## .FromByteStream( tmpS ); } }
#else
#define ABRS(A) ByteStream stream ## A = A.ToByteStream(); BStream.AddStream( #A, stream ## A );
#define GBRS(A) { ByteStream tmpS(true); if ( inStream.GetStream( #A, tmpS ) ) { A.FromByteStream( tmpS ); } }
#endif

#define ABRSV(A) {                                      \
    std::vector<ByteStream> stream ## A;                \
    for ( const auto & a : A ) {                        \
        stream ## A.emplace_back( a.ToByteStream() );   \
    }                                                   \
    BStream.AddStreamVector( #A, stream ## A );         \
}

#define GBRSV(A) {                                      \
    std::vector<ByteStream> stream ## A;                \
    inStream.GetStreamVector( #A, stream ## A );        \
    A.resize( stream ## A.size() );                     \
    for ( int i = 0; i < A.size(); i++ ) {              \
        A[i].FromByteStream( stream ## A[i] );          \
    }                                                   \
}

// Forward declaration
class ByteStream;

#endif /* BYTESTREAMMACROS_H */
