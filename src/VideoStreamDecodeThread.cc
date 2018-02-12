//
// Created by hjzh on 18-2-5.
//

#include "VideoStreamDecodeThread.h"
#include "yuv2bgr.h"

bool VideoStreamDecodeThread::Init() {
  bool res = true;
  m_strStreamUrl = m_config.m_szVideoStreamAddress;
  m_strHwName = m_config.m_szHwName;
  av_register_all();
  avcodec_register_all();
  avformat_network_init();
  avfilter_register_all();
  avdevice_register_all();
  res = Connect();
  return res;
}

ErrCode VideoStreamDecodeThread::Connect() {
  ErrCode res = RES_SUC;
  do {
    // Try to open input
    if (avformat_open_input(&m_pFormatCtx, m_strHwName.c_str(), NULL, NULL) != 0) {
      res = RES_ERR_OPEN_INPUT;
      break;
    }

    // Try to find a stream
    if (avformat_find_stream_info(m_pFormatCtx, NULL) < 0) {
      res = RES_ERR_FIND_STREAM_INFO;
      break;
    }

    // Try to find the video stream
    m_nVideoStreamIndex = av_find_best_stream(m_pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
    if (m_nVideoStreamIndex < 0) {
      res = RES_ERR_FIND_VIDEO_STREAM;
      break;
    }

    // Try to configure for gpu decode
    m_pDecoder = avcodec_find_decoder_by_name("cuda");
    m_pDecoderCtx = avcodec_alloc_context3(m_pDecoder);
    m_pDecoderCtx->height = m_pFormatCtx->streams[m_nVideoStreamIndex]->codecpar->height;
    m_pDecoderCtx->width = m_pFormatCtx->streams[m_nVideoStreamIndex]->codecpar->width;
    m_pDecoderCtx->thread_type = 2;

    if (!m_pDecoder || avcodec_open2(m_pDecoderCtx, m_pDecoder, NULL) < 0) {
      res = RES_NOT_FIND_DECODER;
      break;
    }
  } while (false);
  // If failed in connection, release the resources
  if (res != RES_SUC) {
    avformat_close_input(&m_pFormatCtx);
    if (m_pDecoderCtx) {
      avcodec_free_context(&m_pDecoderCtx);
    }
  } else {
    m_bConnected = true;
  }
  return res;
}

void VideoStreamDecodeThread::Start(const std::function<FrameCBFunc> &cb) {
  m_frameCallback = cb;
  m_thread.start(*this);
}

void VideoStreamDecodeThread::Exit() {
  m_stop = true;
  m_thread.join();
}

void VideoStreamDecodeThread::run() {
  AVFrame *pYUVFrame = av_frame_alloc();
  AVFrame *pBGRFrame = av_frame_alloc();
  AVPacket packet;
  int align = 32;
  int buffer_size = av_image_get_buffer_size(AV_PIX_FMT_BGR24, m_pDecoderCtx->width, m_pDecoderCtx->height, align);
  unsigned char* buffer = (unsigned char*)(av_malloc(buffer_size * sizeof(uint8_t)));
  av_image_fill_arrays(pBGRFrame->data, pBGRFrame->linesize, buffer, AV_PIX_FMT_BGR24, m_pDecoderCtx->width,
                       m_pDecoderCtx->height, align);

  int bufsize0, bufsize1, resolution;
  cv::cuda::GpuMat reqMat, resMat, resizedMat;

  // av_read_play(m_pFormatCtx);//play RTSP
  Poco::Int64 nFrameNum = 0;
  int break_status = 0;
  bool is_first_time = true;
  while (!m_stop)
  {
    if (av_read_frame(m_pFormatCtx, &packet) != 0)
    {
      m_bConnected = false;
      break_status = 1;
      break;
    }

    if (packet.stream_index == m_nVideoStreamIndex)
    {//packet is video
      int got_picture = 0;
      int res = avcodec_decode_video2(m_pDecoderCtx, pYUVFrame, &got_picture, &packet); // TO DO use cuda
      if (res >= 0 && got_picture)
      {
        ++nFrameNum;
        if (m_stop)
        {
          break_status = 2;
          break;
        }
        SharedIplImage pImg;
        pImg = cvCreateImage(cvSize(m_pDecoderCtx->width, m_pDecoderCtx->height), 8, 3);
        if (is_first_time) {
          resolution = pYUVFrame->height * pYUVFrame->width;
          reqMat.create(pYUVFrame->height, pYUVFrame->width, CV_8UC3);
          resMat.create(pYUVFrame->height, pYUVFrame->width, CV_8UC3);
          resMat.step = pBGRFrame->linesize[0];
          bufsize0 = pYUVFrame->height * pYUVFrame->linesize[0];
          bufsize1 = pYUVFrame->height * pYUVFrame->linesize[1] / 2;
          is_first_time = false;
        }
        cudaMemcpy(reqMat.data, pYUVFrame->data[0], bufsize0, cudaMemcpyHostToDevice);
        cudaMemcpy(reqMat.data + bufsize0, pYUVFrame->data[1], bufsize1, cudaMemcpyHostToDevice);
        cvtColor(reqMat.data, resMat.data, resolution, pYUVFrame->height, pYUVFrame->width, pYUVFrame->linesize[0]);
        cudaMemcpy(pImg->imageData, resMat.data, resMat.cols *resMat.rows * sizeof(uchar3), cudaMemcpyDeviceToHost);
        cv::Mat img = cv::cvarrToMat(pImg, true);
        ImageFrame data(img);
        data.frameIdx = nFrameNum;
        m_frameCallback(data);
      }
    }
    av_free_packet(&packet);
  }
  av_read_pause(m_pFormatCtx);
  av_frame_free(&pYUVFrame);
  av_frame_free(&pBGRFrame);
  av_free(buffer);

  avcodec_free_context(&m_pDecoderCtx);
  avformat_close_input(&m_pFormatCtx);
}