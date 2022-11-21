import { registerRpcEvent } from './handlers'

interface DTSImage {
  width: number
  height: number
  image_data: {
    type: 'Buffer'
    data: number[]
  }
}

function resizeImage(
  url: string,
  width = 248,
  height = 248
): Promise<DTSImage> {
  return new Promise((resolve, reject) => {
    if (!url.startsWith('spotify:')) {
      if (url.startsWith('https://i.scdn.co/image/')) {
        const imageId = url.substring('https://i.scdn.co/image/'.length)
        url = 'spotify:image:' + imageId
      } else {
        console.log('unknown url format', url)
      }
    }

    const image = new Image()
    const canvas = document.createElement('canvas')
    const ctx = canvas.getContext('2d')!

    canvas.width = width
    canvas.height = height

    ctx.imageSmoothingEnabled = true
    ctx.imageSmoothingQuality = 'high'

    image.addEventListener('load', () => {
      ctx.drawImage(image, 0, 0, image.width, image.height, 0, 0, width, height)

      const dataURL = canvas.toDataURL('image/jpeg', 0.8)

      let data = [...atob(dataURL.substring(dataURL.indexOf(',') + 1))].map(i =>
        i.charCodeAt(0)
      )

      let di: DTSImage = {
        width,
        height,
        image_data: {
          type: 'Buffer',
          data: data,
        },
      }
      resolve(di)
    })
    image.src = url
  })
}

const DEFAULT_WIDTH_HEIGHT = 248
const THUMBNAIL_WIDTH_HEIGHT = 96

registerRpcEvent<{ id: string }>(
  'com.spotify.get_image',
  async ({ argsKw: { id }, reply }) => {
    console.log('Requesting image')
    const resized = await resizeImage(
      id,
      DEFAULT_WIDTH_HEIGHT,
      DEFAULT_WIDTH_HEIGHT
    )
    console.log('Got image', resized)
    reply(resized)
  }
)

registerRpcEvent<{ id: string }>(
  'com.spotify.get_thumbnail_image',
  async ({ argsKw: { id }, reply }) => {
    console.log('Requesting image')
    const resized = await resizeImage(
      id,
      THUMBNAIL_WIDTH_HEIGHT,
      THUMBNAIL_WIDTH_HEIGHT
    )
    console.log('Got image', resized)
    reply(resized)
  }
)
