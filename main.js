const { app, BrowserWindow } = require('electron/main')
const { screen } = require('electron')
const { globalShortcut } = require('electron')

const createWindow = () => {
const { width, height } = screen.getPrimaryDisplay().workAreaSize

const win = new BrowserWindow({
    width: Math.floor(width * 0.8),
    height: Math.floor(height * 0.8),
    frame: false, // Hide the frames
    transparent: true
})

  win.loadFile('src/index.html')
}

app.whenReady().then(() => {
    createWindow()

    app.on('activate', () => {
        if (BrowserWindow.getAllWindows().length === 0) {
            createWindow()
        }
    })

    // Listen for 'esc' key in all windows to close them
    globalShortcut.register('Escape', () => {
        const windows = BrowserWindow.getAllWindows()
        windows.forEach(win => win.close())
    })
})

app.on('will-quit', () => {
    globalShortcut.unregisterAll()
})

app.on('window-all-closed', () => {
  if (process.platform !== 'darwin') {
    app.quit()
  }
})