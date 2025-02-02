// server.js

const express = require('express');
const mongoose = require('mongoose');
const cors = require('cors');

// 1. Inlined MongoDB connection string
const MONGO_URI = 'mongodb+srv://GooDMaN:TqktaUPJc3V4qKB5@cluster0.rtmsg.mongodb.net/?retryWrites=true&w=majority&appName=Cluster0';

// 2. Create an Express app
const app = express();
app.use(cors());
app.use(express.json());

// 3. Connect to MongoDB
mongoose
  .connect(MONGO_URI, {
    useNewUrlParser: true,
    useUnifiedTopology: true
  })
  .then(() => {
    console.log('Connected to MongoDB!');
  })
  .catch((err) => {
    console.error('MongoDB connection error:', err);
  });

// 4. Define a simple Mongoose schema/model
const PostSchema = new mongoose.Schema({
  title: String,
  description: String
}, { timestamps: true });

const Post = mongoose.model('Post', PostSchema);

// 5. Define an endpoint to return all posts
app.get('/api/posts/all-posts', async (req, res) => {
  try {
    const posts = await Post.find();
    res.json({
      success: true,
      message: 'posts',
      data: posts
    });
  } catch (error) {
    console.error('Error fetching posts:', error);
    res.status(500).json({
      success: false,
      message: 'Server error',
      error: error.message
    });
  }
});

// 6. Start the server (port 5001)
const PORT = 5001;
app.listen(PORT, () => {
  console.log(`Server is running on port ${PORT}`);
});