// server.js

const express = require('express');
const mongoose = require('mongoose');
const cors = require('cors');

const app = express();
app.use(cors());
app.use(express.json());

// MongoDB connection string (update as needed)
const MONGO_URI =
  'mongodb+srv://GooDMaN:TqktaUPJc3V4qKB5@cluster0.rtmsg.mongodb.net/?retryWrites=true&w=majority&appName=Cluster0';


mongoose
  .connect(MONGO_URI, {
    useNewUrlParser: true,
    useUnifiedTopology: true
  })
  .then(() => console.log('Connected to MongoDB!'))
  .catch((err) => console.error('MongoDB connection error:', err));

// Define the Mongoose schema/model.
// The "description" field stores a JSON string that includes patient details and a "visits" array.

const PostSchema = new mongoose.Schema(
  {
    title: String,
    description: String, // JSON string (patient details, including visits array)
    userId: String
  },
  { timestamps: true }
);
const Post = mongoose.model('Post', PostSchema);

// =======================================
// GET /api/posts/search?healthCard=1
// =======================================
// This endpoint searches for a patient using their health card number.
app.get('/api/posts/search', async (req, res) => {
  const { healthCard } = req.query;
  console.log("Searching for healthCard:", healthCard);
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

// Start the server (port 5001)
const PORT = 5001;
app.listen(PORT, () => {
  console.log(`Server is running on port ${PORT}`);
});